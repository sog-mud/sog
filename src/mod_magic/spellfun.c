/*
 * $Id: spellfun.c,v 1.294 2002-08-02 13:10:39 tatyana Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <merc.h>

#include <sog.h>
#include <quest.h>

#include <magic.h>
#include "magic_impl.h"

DECLARE_SPELL_FUN(generic_damage_spellfun);

DECLARE_SPELL_FUN(spell_armor);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_chain_lightning);
DECLARE_SPELL_FUN(spell_charm_person);
DECLARE_SPELL_FUN(spell_chill_touch);
DECLARE_SPELL_FUN(spell_colour_spray);
DECLARE_SPELL_FUN(spell_continual_light);
DECLARE_SPELL_FUN(spell_create_rose);
DECLARE_SPELL_FUN(spell_detect_hidden);
DECLARE_SPELL_FUN(spell_detect_fade);
DECLARE_SPELL_FUN(spell_detect_invis);
DECLARE_SPELL_FUN(spell_detect_magic);
DECLARE_SPELL_FUN(spell_detect_charm);
DECLARE_SPELL_FUN(spell_true_seeing);
DECLARE_SPELL_FUN(spell_enchant_armor);
DECLARE_SPELL_FUN(spell_enchant_weapon);
DECLARE_SPELL_FUN(spell_energy_drain);
DECLARE_SPELL_FUN(spell_mana_drain);
DECLARE_SPELL_FUN(spell_draining_touch);
DECLARE_SPELL_FUN(spell_iceball);
DECLARE_SPELL_FUN(spell_fireproof);
DECLARE_SPELL_FUN(spell_faerie_fire);
DECLARE_SPELL_FUN(spell_faerie_fog);
DECLARE_SPELL_FUN(spell_floating_disc);
DECLARE_SPELL_FUN(spell_levitation);
DECLARE_SPELL_FUN(spell_gate);
DECLARE_SPELL_FUN(spell_giant_strength);
DECLARE_SPELL_FUN(spell_haste);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_improved_identify);
DECLARE_SPELL_FUN(spell_infravision);
DECLARE_SPELL_FUN(spell_invisibility);
DECLARE_SPELL_FUN(spell_locate_object);
DECLARE_SPELL_FUN(spell_magic_missile);
DECLARE_SPELL_FUN(spell_mass_invis);
DECLARE_SPELL_FUN(spell_pass_door);
DECLARE_SPELL_FUN(spell_plague);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_recharge);
DECLARE_SPELL_FUN(spell_shield);
DECLARE_SPELL_FUN(spell_shocking_grasp);
DECLARE_SPELL_FUN(spell_sleep);
DECLARE_SPELL_FUN(spell_slow);
DECLARE_SPELL_FUN(spell_stone_skin);
DECLARE_SPELL_FUN(spell_summon);
DECLARE_SPELL_FUN(spell_teleport);
DECLARE_SPELL_FUN(spell_bamf);
DECLARE_SPELL_FUN(spell_ventriloquate);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_find_object);
DECLARE_SPELL_FUN(spell_lightning_shield);
DECLARE_SPELL_FUN(spell_shocking_trap);
DECLARE_SPELL_FUN(spell_spectral_furor);
DECLARE_SPELL_FUN(spell_disruption);
DECLARE_SPELL_FUN(spell_sonic_resonance);
DECLARE_SPELL_FUN(spell_sulfurus_spray);
DECLARE_SPELL_FUN(spell_caustic_font);
DECLARE_SPELL_FUN(spell_acetum_primus);
DECLARE_SPELL_FUN(spell_galvanic_whip);
DECLARE_SPELL_FUN(spell_magnetic_thrust);
DECLARE_SPELL_FUN(spell_quantum_spike);
DECLARE_SPELL_FUN(spell_hand_of_undead);
DECLARE_SPELL_FUN(spell_astral_walk);
DECLARE_SPELL_FUN(spell_helical_flow);
DECLARE_SPELL_FUN(spell_corruption);
DECLARE_SPELL_FUN(spell_hurricane);
DECLARE_SPELL_FUN(spell_detect_undead);
DECLARE_SPELL_FUN(spell_take_revenge);
DECLARE_SPELL_FUN(spell_portal);
DECLARE_SPELL_FUN(spell_nexus);
DECLARE_SPELL_FUN(spell_disintegrate);
DECLARE_SPELL_FUN(spell_bark_skin);
DECLARE_SPELL_FUN(spell_ranger_staff);
DECLARE_SPELL_FUN(spell_transform);
DECLARE_SPELL_FUN(spell_mana_transfer);
DECLARE_SPELL_FUN(spell_mental_knife);
DECLARE_SPELL_FUN(spell_demon_summon);
DECLARE_SPELL_FUN(spell_scourge);
DECLARE_SPELL_FUN(spell_manacles);
DECLARE_SPELL_FUN(spell_shield_of_ruler);
DECLARE_SPELL_FUN(spell_guard_call);
DECLARE_SPELL_FUN(spell_nightwalker);
DECLARE_SPELL_FUN(spell_eyes_of_intrigue);
DECLARE_SPELL_FUN(spell_shadow_cloak);
DECLARE_SPELL_FUN(spell_nightfall);
DECLARE_SPELL_FUN(spell_garble);
DECLARE_SPELL_FUN(spell_confuse);
DECLARE_SPELL_FUN(spell_terangreal);
DECLARE_SPELL_FUN(spell_kassandra);
DECLARE_SPELL_FUN(spell_sebat);
DECLARE_SPELL_FUN(spell_matandra);
DECLARE_SPELL_FUN(spell_amnesia);
DECLARE_SPELL_FUN(spell_chaos_blade);
DECLARE_SPELL_FUN(spell_stalker);
DECLARE_SPELL_FUN(spell_tesseract);
DECLARE_SPELL_FUN(spell_brew);
DECLARE_SPELL_FUN(spell_shadowlife);
DECLARE_SPELL_FUN(spell_ruler_badge);
DECLARE_SPELL_FUN(spell_remove_badge);
DECLARE_SPELL_FUN(spell_dragon_strength);
DECLARE_SPELL_FUN(spell_blue_dragon);
DECLARE_SPELL_FUN(spell_green_dragon);
DECLARE_SPELL_FUN(spell_white_dragon);
DECLARE_SPELL_FUN(spell_black_dragon);
DECLARE_SPELL_FUN(spell_red_dragon);
DECLARE_SPELL_FUN(spell_dragon_plate);
DECLARE_SPELL_FUN(spell_squire);
DECLARE_SPELL_FUN(spell_dragon_weapon);
DECLARE_SPELL_FUN(spell_entangle);
DECLARE_SPELL_FUN(spell_holy_armor);
DECLARE_SPELL_FUN(spell_love_potion);
DECLARE_SPELL_FUN(spell_protective_shield);
DECLARE_SPELL_FUN(spell_deafen);
DECLARE_SPELL_FUN(spell_disperse);
DECLARE_SPELL_FUN(spell_honor_shield);
DECLARE_SPELL_FUN(spell_dragons_breath);
DECLARE_SPELL_FUN(spell_sand_storm);
DECLARE_SPELL_FUN(spell_scream);
DECLARE_SPELL_FUN(spell_attract_other);
DECLARE_SPELL_FUN(spell_animate_dead);
DECLARE_SPELL_FUN(spell_bone_dragon);
DECLARE_SPELL_FUN(spell_enhanced_armor);
DECLARE_SPELL_FUN(spell_meld_into_stone);
DECLARE_SPELL_FUN(spell_web);
DECLARE_SPELL_FUN(spell_mend);
DECLARE_SPELL_FUN(spell_shielding);
DECLARE_SPELL_FUN(spell_link);
DECLARE_SPELL_FUN(spell_power_word_kill);
DECLARE_SPELL_FUN(spell_eyed_sword);
DECLARE_SPELL_FUN(spell_lion_help);
DECLARE_SPELL_FUN(spell_magic_jar);
DECLARE_SPELL_FUN(spell_fear);
DECLARE_SPELL_FUN(spell_protection_heat);
DECLARE_SPELL_FUN(spell_protection_cold);
DECLARE_SPELL_FUN(spell_fire_shield);
DECLARE_SPELL_FUN(spell_cold_shield);
DECLARE_SPELL_FUN(spell_witch_curse);
DECLARE_SPELL_FUN(spell_knock);
DECLARE_SPELL_FUN(spell_hallucination);
DECLARE_SPELL_FUN(spell_wolf);
DECLARE_SPELL_FUN(spell_dragon_skin);
DECLARE_SPELL_FUN(spell_insanity);
DECLARE_SPELL_FUN(spell_power_stun);
DECLARE_SPELL_FUN(spell_improved_invis);
DECLARE_SPELL_FUN(spell_randomizer);
DECLARE_SPELL_FUN(spell_flesh_golem);
DECLARE_SPELL_FUN(spell_stone_golem);
DECLARE_SPELL_FUN(spell_iron_golem);
DECLARE_SPELL_FUN(spell_adamantite_golem);
DECLARE_SPELL_FUN(spell_mysterious_dream);
DECLARE_SPELL_FUN(spell_ratform);
DECLARE_SPELL_FUN(spell_polymorph);
DECLARE_SPELL_FUN(spell_deathen);
DECLARE_SPELL_FUN(spell_lich);
DECLARE_SPELL_FUN(spell_protection_negative);
DECLARE_SPELL_FUN(spell_ruler_aura);
DECLARE_SPELL_FUN(spell_evil_spirit);
DECLARE_SPELL_FUN(spell_disgrace);
DECLARE_SPELL_FUN(spell_control_undead);
DECLARE_SPELL_FUN(spell_assist);
DECLARE_SPELL_FUN(spell_summon_shadow);
DECLARE_SPELL_FUN(spell_farsight);
DECLARE_SPELL_FUN(spell_mirror);
DECLARE_SPELL_FUN(spell_doppelganger);
DECLARE_SPELL_FUN(spell_hunger_weapon);
DECLARE_SPELL_FUN(spell_enlarge);
DECLARE_SPELL_FUN(spell_shrink);
DECLARE_SPELL_FUN(spell_water_breathing);
DECLARE_SPELL_FUN(spell_blur);
DECLARE_SPELL_FUN(spell_find_familiar);
DECLARE_SPELL_FUN(spell_death_ripple);
DECLARE_SPELL_FUN(spell_simulacrum);
DECLARE_SPELL_FUN(spell_misleading);
DECLARE_SPELL_FUN(spell_phantasmal_force);
DECLARE_SPELL_FUN(spell_protection_from_missiles);
DECLARE_SPELL_FUN(spell_alarm);
DECLARE_SPELL_FUN(spell_globe_of_invulnerability);
DECLARE_SPELL_FUN(spell_cloak_of_leaves);

SPELL_FUN(generic_damage_spellfun, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_armor, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already armored.", ch);
		else {
			act("$N is already armored.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	 = level;
	paf->duration	 = 7 + level / 6;
	/* paf->modifier  = -20;*/
	paf->modifier	 = -1 * UMAX(20, 10 + level / 4);
	INT(paf->location) = APPLY_AC;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel someone protecting you.", victim);
	if (ch != victim) {
		act("$N is protected by your magic.",
		    ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(spell_blindness, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_BLIND)
	||  saves_spell(level, victim, DAM_OTHER))  {
		act_char("You failed.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -4;
	paf->duration	= 3 + level / 15;
	paf->bitvector	= AFF_BLIND;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are blinded!", victim);
	act("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
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
	{ "levitation",		"$n falls to the ground!"		},
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

	{ NULL, NULL }
};

static bool
dispel(CHAR_DATA *victim, int level)
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

SPELL_FUN(spell_dispel_magic, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool found = FALSE;

	/*
	 * check saves if victim doesn't trust ch.
	 */
	if (!check_trust(ch, victim) && saves_spell(level, victim, DAM_OTHER)) {
		act_char("You feel a brief tingling sensation.", victim);
		act_char("You failed.", ch);
		return;
	}

	found = dispel(victim, level);

	if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&&  !saves_dispel(level, LEVEL(victim), -1)
	&&  !is_sn_affected(victim, "sanctuary")) {
		REMOVE_BIT(victim->affected_by, AFF_SANCTUARY);
		act("The white aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)
	&&  !saves_dispel(level, LEVEL(victim), -1)
	&&  !is_sn_affected(victim, "black shroud")) {
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

	dam = calc_spell_damage(ch, *plevel, sn);
	dam = dam * 2 / 3;
	if (saves_spell(*plevel, vch, DAM_LIGHTNING))
		dam /= 3;
	damage(ch, vch, dam, sn, DAM_F_SHOW);
	*plevel -= 4;  /* decrement damage */

	if (*plevel <= 0)
		return vch;

	return NULL;
}

SPELL_FUN(spell_chain_lightning, sn, level, ch, vo)
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

	dam = calc_spell_damage(ch, level, sn);
	dam = dam * 2 / 3;
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
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
		dam = calc_spell_damage(ch, level, sn);
		dam = dam * 2 / 3;
		if (saves_spell(level, ch, DAM_LIGHTNING))
			dam /= 3;
		damage(ch, ch, dam, sn, DAM_F_SHOW);
		level -= 4;  /* decrement damage */
	} /* now go back and find more victims */
}

SPELL_FUN(spell_charm_person, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

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

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= number_fuzzy(level / 5);
	paf->bitvector	= AFF_CHARM;
	affect_to_char(victim, paf);
	aff_free(paf);

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

SPELL_FUN(spell_chill_touch, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = calc_spell_damage(ch, level, sn);

	if (!saves_spell(level, victim, DAM_COLD)) {
		AFFECT_DATA *paf;

		act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= 6;
		INT(paf->location)= APPLY_STR;
		paf->modifier	= -1;
		affect_join(victim, paf);
		aff_free(paf);
	} else
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(spell_colour_spray, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = calc_spell_damage(ch, level, sn);
	if (saves_spell(level, victim, DAM_LIGHT))
		dam /= 2;
	else
		spellfun_call("blindness", NULL, level/2, ch, (void *) victim);

	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

#define OBJ_VNUM_LIGHT_BALL		21

SPELL_FUN(spell_continual_light, sn, level, ch, vo)
{
	OBJ_DATA *light;

	if (target_name[0] != '\0') { /* do a glow on some object */
		light = get_obj_carry(ch, ch, target_name);

		if (light == NULL) {
			act_char("You don't see that here.", ch);
			return;
		}

		if (IS_OBJ_STAT(light, ITEM_GLOW)) {
			act("$p is already glowing.", ch, light, NULL, TO_CHAR);
			return;
		}

		SET_OBJ_STAT(light, ITEM_GLOW);
		act("$p glows with a white light.", ch, light, NULL, TO_ALL);
		return;
	}

	if ((light = create_obj(OBJ_VNUM_LIGHT_BALL, 0)) == NULL)
		return;

	obj_to_room(light, ch->in_room);
	act("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
	act("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
}

#define OBJ_VNUM_ROSE			19

SPELL_FUN(spell_create_rose, sn, level, ch, vo)
{
	OBJ_DATA *rose = create_obj(OBJ_VNUM_ROSE, 0);
	if (rose == NULL)
		return;

	act("$n has created $p.", ch, rose, NULL, TO_ROOM);
	act("You create $p.", ch, rose, NULL, TO_CHAR);
	obj_to_char(rose, ch);
}

SPELL_FUN(spell_detect_hidden, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_HIDDEN)) {
		if (victim == ch)
			act_char("You are already as alert as you can be.", ch);
		else {
			act("$N can already sense hidden lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= 5 + level / 3;
	paf->bitvector	= ID_HIDDEN;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your awareness improves.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_detect_fade, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_FADE)) {
		if (victim == ch)
			act_char("You are already as alert as you can be.", ch);
		else {
			act("$N can already sense faded lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= 5 + level / 3;
	paf->bitvector	= ID_FADE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your awareness improves.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_detect_invis, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_INVIS)) {
		if (victim == ch)
			act_char("You can already see invisible.", ch);
		else {
			act("$N can already see invisible things.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_INVIS;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_detect_charm, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_CHARM)) {
		if (victim == ch)
			act_char(
			    "You can already distinguish charmed creatures.", ch);
		else {
			act("$N can already distinguish charmed creatures.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_CHARM;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Now you are able to detect charmed creatures.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_detect_magic, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_MAGIC)) {
		if (victim == ch)
			act_char("You can already sense magical auras.", ch);
		else {
			act("$N can already detect magic.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_MAGIC;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_true_seeing, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_TRUESEEING)) {
		if (victim == ch)
			act_char("You can already see everything.", ch);
		else {
			act("$N can already see invisible things.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (2 + level / 5);
	paf->bitvector	= ID_TRUESEEING;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Now you have a god-like sight.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_enchant_armor, sn, level, ch, vo)
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

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		for (paf = obj->pObjIndex->affected;
		     paf != NULL;
		     paf = paf->next) {
			if (!IS_APPLY_AFFECT(paf))
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
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (!IS_APPLY_AFFECT(paf))
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
			if (!IS_APPLY_AFFECT(paf))
				continue;

			if (INT(paf->location) == APPLY_AC) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier += added;
				paf->level = UMAX(paf->level, level);
			}
		}
	} else { /* add a new affect */
		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_AC;
		paf->modifier	= added;
		affect_to_obj(obj, paf);
		aff_free(paf);
	}

	if (hp_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (!IS_APPLY_AFFECT(paf))
				continue;

			if (INT(paf->location) == APPLY_HIT) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier -= added;
				paf->level = UMAX(paf->level,level);
			}
		}
	} else { /* add a new affect */
		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_HIT;
		paf->modifier	= -added;
		affect_to_obj(obj, paf);
		aff_free(paf);
	}
}

SPELL_FUN(spell_enchant_weapon, sn, level, ch, vo)
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

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		for (paf = obj->pObjIndex->affected; paf != NULL; paf = paf->next) {
			if (!IS_APPLY_AFFECT(paf))
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
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (!IS_APPLY_AFFECT(paf))
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
		act("$p shivers violently and explodes!",
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
			if (!IS_APPLY_AFFECT(paf))
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
		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_DAMROLL;
		paf->modifier	= added;
		affect_to_obj(obj, paf);
		aff_free(paf);
	}

	if (hit_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (!IS_APPLY_AFFECT(paf))
				continue;

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
		paf = aff_new(TO_OBJECT, sn);
		paf->level      = level;
		paf->duration   = -1;
		INT(paf->location) = APPLY_HITROLL;
		paf->modifier   = added;
		affect_to_obj(obj, paf);
		aff_free(paf);
	}
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
SPELL_FUN(spell_energy_drain, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
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
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level		= level/2;
		paf->duration		= 6+level/12;
		INT(paf->location)	= APPLY_LEVEL;
		paf->modifier		= -1;
		affect_join(victim, paf);
		aff_free(paf);
	}

	act_char("You feel your life slipping away!", victim);
	act_char("Wow....what a rush!", ch);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(spell_mana_drain, sn, level, ch, vo)
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
	if (saves_spell(level, victim, DAM_NEGATIVE))
		manadam /=3;
	manadam = UMIN(manadam, victim->mana);
	victim->mana -= manadam;
	ch->mana += manadam/2;
}

SPELL_FUN(spell_draining_touch, sn, level, ch, vo)
{
	CHAR_DATA *victim=(CHAR_DATA *)vo;
	int dam, gdam;

	gdam = dam = dice (level, 3) + 10;
	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		dam /= 3;
		gdam = 0;
	}

	ch->hit += gdam;
	act("$n touches $N with $s fingers.", ch, NULL, victim, TO_NOTVICT);
	act("$n touches you with $s fingers.", ch, NULL, victim, TO_VICT);

	damage(ch, victim, dam, sn, DAM_F_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_sn_affected(victim, sn)
	&&  !saves_spell(level-5, victim, DAM_NEGATIVE)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= 1;
		INT(paf->location)= APPLY_LEVEL;
		paf->modifier	= -1*level/15;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("You feel drained!", victim, NULL, NULL, TO_VICT);
	}
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
	damage(ch, vch, *pdam, sn, DAM_F_SHOW);
	return NULL;
}

SPELL_FUN(spell_iceball, sn, level, ch, vo)
{
	int dam = calc_spell_damage(ch, level, sn);
	dam = dam * 4 / 5;
	vo_foreach(ch->in_room, &iter_char_room, iceball_cb,
		   sn, level, ch, &dam, number_range(level, 2 * level));
}

SPELL_FUN(spell_fireproof, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("$p is already protected from burning.",
		    ch, obj, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= number_fuzzy(level / 4);
	paf->bitvector	= ITEM_BURN_PROOF;
	affect_to_obj(obj, paf);
	aff_free(paf);

	act("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
	act("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}

SPELL_FUN(spell_faerie_fire, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 10 + level / 5;
	INT(paf->location)= APPLY_AC;
	paf->modifier	= 2 * level;
	paf->bitvector	= AFF_FAERIE_FIRE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are surrounded by a pink outline.", victim);
	act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_faerie_fog, sn, level, ch, vo)
{
	CHAR_DATA *ich;

	act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	act_char("You conjure a cloud of purple smoke.", ch);

	for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
		if (ich->invis_level > 0)
			continue;

		if (ich == ch || saves_spell(level, ich, DAM_AIR))
			continue;

		affect_bit_strip(ich, TO_INVIS, ID_ALL_INVIS);
		REMOVE_BIT(ich->affected_by, ID_ALL_INVIS);

		act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
		act_char("You are revealed!", ich);
	}
}

#define OBJ_VNUM_DISC			23

SPELL_FUN(spell_floating_disc, sn, level, ch, vo)
{
	OBJ_DATA *disc, *floating;
	AFFECT_DATA *paf;

	floating = get_eq_char(ch, WEAR_FLOAT);
	if (floating != NULL && IS_OBJ_STAT(floating, ITEM_NOREMOVE)) {
		act("You can't remove $p.", ch, floating, NULL, TO_CHAR);
		return;
	}

	disc = create_obj(OBJ_VNUM_DISC, 0);
	if (disc == NULL)
		return;

	INT(disc->value[0]) = level * 10; /* 10 pounds per level capacity */
	INT(disc->value[3]) = level * 5; /* 5 pounds per level max per item */
	disc->timer	= level / 2 - number_range(0, level / 4);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= disc->timer;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n has created $p.", ch, disc, NULL, TO_ROOM);
	act("You create $p.", ch, disc, NULL, TO_CHAR);
	obj_to_char(disc, ch);
	equip_char(ch, disc, WEAR_FLOAT);
}

SPELL_FUN(spell_levitation, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_FLYING)
	||  is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already flying.", ch);
		else {
			act("$N doesn't need your help to levitation.",
			    ch,NULL,victim,TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level + 3;
	paf->bitvector	= AFF_FLYING;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You rise off the ground.", victim);
	act("$n rises off the ground.", victim, NULL, NULL, TO_ROOM);
}

static inline void
gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	teleport_char(ch, NULL, victim->in_room,
		      "$n steps through a gate and vanishes.",
		      "You step through a gate and vanish.",
		      "$n has arrived through a gate.");
}

SPELL_FUN(spell_gate, sn, level, ch, vo)
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

SPELL_FUN(spell_giant_strength, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn))
	{
		if (victim == ch)
		  act_char("You are already as strong as you can get!", ch);
		else
		  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (10 + level / 3);
	INT(paf->location)= APPLY_STR;
	paf->modifier	= UMAX(2,level / 10);
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your muscles surge with heightened power!", victim);
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
}

/* RT haste spell */

SPELL_FUN(spell_haste, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)
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

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	if (victim == ch)
		paf->duration  = level/2;
	else
		paf->duration  = level/4;
	INT(paf->location) = APPLY_DEX;
	paf->modifier  = UMAX(2,level / 12);
	paf->bitvector = AFF_HASTE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel yourself moving more quickly.", victim);
	act("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_identify, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	BUFFER *output;

	if (OBJ_IS(obj, OBJ_NOIDENT)) {
		act("You failed to determine the true nature of the $P.",
		    ch, NULL, obj, TO_CHAR);
	}

	output = buf_new(GET_LANG(ch));
	format_obj(output, obj);
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
				   FOA_F_NODURATION | FOA_F_NOAFFECTS);
	format_obj_affects(output, obj->affected, FOA_F_NOAFFECTS);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

SPELL_FUN(spell_improved_identify, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	BUFFER *output;

	if (OBJ_IS(obj, OBJ_NOIDENT)) {
		act("You failed to determine the true nature of the $P.",
		    ch, NULL, obj, TO_CHAR);
	}

	output = buf_new(0);
	format_obj(output, obj);
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
					FOA_F_NODURATION);
	format_obj_affects(output, obj->affected, 0);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

SPELL_FUN(spell_infravision, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

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
	act("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= 2 * level;
	paf->bitvector	= ID_INFRARED;
	affect_to_char(victim, paf);
	aff_free(paf);
}

SPELL_FUN(spell_invisibility, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	CHAR_DATA *victim;

	/* object invisibility */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_INVIS)) {
			act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
			return;
		}

		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= level / 4 + 12;
		paf->bitvector	= ITEM_INVIS;
		affect_to_obj(obj, paf);
		aff_free(paf);

		act("$p fades out of sight.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;

	if (HAS_INVIS(victim, ID_INVIS))
		return;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
	act_char("You fade out of existence.", victim);

	paf = aff_new(TO_INVIS, sn);
	paf->level	= level;
	paf->duration	= (level / 8 + 10);
	paf->bitvector	= ID_INVIS;
	affect_to_char(victim, paf);
	aff_free(paf);
}

SPELL_FUN(spell_locate_object, sn, level, ch, vo)
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
			buffer = buf_new(0);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(ch, in_obj->carried_by)) {
			buf_printf(buffer, BUF_END, "One is carried by %s\n",
			    PERS(in_obj->carried_by, ch, GET_LANG(ch), ACT_FORMSH));
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

SPELL_FUN(spell_magic_missile, sn, level, ch, vo)
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

	if (is_sn_affected(victim, "protective shield"))  {
		int count = 1;

		count += LEVEL(ch) > 4 ? 1 : 0;
		count += LEVEL(ch) > 8 ? 1 : 0;
		count += LEVEL(ch) > 12 ? 1 : 0;
		count += LEVEL(ch) > 16 ? 1 : 0;

		act("Your magic $qj{missile} fizzle out near your victim.",
		    ch, (const void *) count, NULL, TO_CHAR);
		act("Your shield blocks $N's magic $qj{missile}.",
		    victim, (const void *) count, ch, TO_CHAR);
		return;
	}

	level = UMIN(level, (int) (sizeof(dam_each)/sizeof(dam_each[0])) - 1);
	level = UMAX(0, level);
	if (LEVEL(ch) > 50)
		dam = level / 4;
	else
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_ENERGY))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_F_SHOW);

	if (LEVEL(ch) > 4)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim, DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_F_SHOW);
	}
	if (LEVEL(ch) > 8)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_F_SHOW);
	}
	if (LEVEL(ch) > 12)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_F_SHOW);
	}
	if (LEVEL(ch) > 16)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_F_SHOW);
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

SPELL_FUN(spell_mass_invis, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	paf = aff_new(TO_INVIS, sn);
	paf->level	= level/2;
	paf->duration	= 24;
	paf->bitvector	= ID_INVIS;
	vo_foreach(ch->in_room, &iter_char_room, mass_invis_cb, ch, paf);
	aff_free(paf);

	act_char("Ok.", ch);
}

SPELL_FUN(spell_pass_door, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_PASS_DOOR)) {
		if (victim == ch)
			act("You are already out of phase.",
			    ch, NULL, NULL, TO_CHAR);
		else
			act("$N is already shifted out of phase.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= number_fuzzy(level / 4);
	paf->bitvector	= AFF_PASS_DOOR;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	act("You turn translucent.", victim, NULL, NULL, TO_CHAR);
}

/* RT plague spell, very nasty */

SPELL_FUN(spell_plague, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (saves_spell(level, victim, DAM_DISEASE)
	|| IS_SET(victim->form, FORM_CONSTRUCT)
	|| IS_SET(victim->form, FORM_UNDEAD)) {
		if (ch->in_room != victim->in_room)
			return;

		if (ch == victim) {
			act("You seem to be unaffected.",
			    ch, NULL, victim, TO_CHAR);
		} else {
			act("$N seems to be unaffected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level * 3/4;
	paf->duration	= (10 + level / 10);
	INT(paf->location)= APPLY_STR;
	paf->modifier	= -1 * UMAX(1,3 + level / 15);
	paf->bitvector	= AFF_PLAGUE;
	affect_join(victim, paf);
	aff_free(paf);

	act_char("You scream in agony as plague sores erupt from your skin.", victim);
	act("$n screams in agony as plague sores erupt from $s skin.",
		victim,NULL,NULL,TO_ROOM);
}

SPELL_FUN(spell_poison, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (obj->item_type == ITEM_FOOD
		||  obj->item_type == ITEM_DRINK_CON) {
			if (IS_OBJ_STAT(obj, ITEM_BLESS)
			||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
				act("Your spell fails to corrupt $p.",
				    ch, obj, NULL, TO_CHAR);
				return;
			}
			INT(obj->value[3]) = 1;
			act("$p is infused with poisonous vapors.",
			    ch, obj, NULL, TO_ALL);
			return;
		}

		if (obj->item_type == ITEM_WEAPON) {
			if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
			||  IS_WEAPON_STAT(obj, WEAPON_FROST)
			||  IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
			||  IS_WEAPON_STAT(obj, WEAPON_SHARP)
			||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
			||  IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
			||  IS_WEAPON_STAT(obj, WEAPON_HOLY)
			||  IS_OBJ_STAT(obj, ITEM_BLESS)
			||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
				act("You can't seem to envenom $p.",
				    ch, obj, NULL, TO_CHAR);
				return;
			}

			if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
				act("$p is already envenomed.",
				    ch, obj, NULL, TO_CHAR);
				return;
			}

			paf = aff_new(TO_WEAPON, sn);
			paf->level	= level / 2;
			paf->duration	= level/8;
			paf->bitvector	= WEAPON_POISON;
			affect_to_obj(obj, paf);
			aff_free(paf);

			act("$p is coated with deadly venom.",
			    ch, obj, NULL, TO_ALL);
			return;
		}

		act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (saves_spell(level, victim, DAM_POISON))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = (10 + level / 10);
	INT(paf->location) = APPLY_STR;
	paf->modifier  = -2;
	paf->bitvector = AFF_POISON;
	affect_join(victim, paf);
	aff_free(paf);

	act_char("You feel very sick.", victim);
	act("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_recharge, sn, level, ch, vo)
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

SPELL_FUN(spell_shield, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already shielded from harm.", ch);
		else {
			act("$N is already protected by a shield.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = (8 + level / 3);
	INT(paf->location) = APPLY_AC;
	paf->modifier  = -1 * UMAX(20, 10 + level/3);
	/* paf->modifier  = -20;*/
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	act_char("You are surrounded by a force shield.", victim);
}

SPELL_FUN(spell_shocking_grasp, sn, level, ch, vo)
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

	level	= UMIN(level, (int) (sizeof(dam_each)/sizeof(dam_each[0])) - 1);
	level	= UMAX(0, level);
		if (LEVEL(ch) > 50)
	dam	= level / 2 ;
		else
	dam		= number_range(dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell(level, victim,DAM_LIGHTNING))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(spell_sleep, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_SLEEP)
	||  IS_SET(victim->form, FORM_UNDEAD)
	||  IS_SET(victim->form, FORM_CONSTRUCT)
	||  saves_spell(level, victim, DAM_CHARM))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 1 + level/10;
	paf->bitvector	= AFF_SLEEP;
	affect_join(victim, paf);
	aff_free(paf);

	if (IS_AWAKE(victim)) {
		act_char("You feel very sleepy ..... zzzzzz.", victim);
		act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
}

SPELL_FUN(spell_slow, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn) || IS_AFFECTED(victim,AFF_SLOW)) {
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

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = (4 + level / 12);
	INT(paf->location) = APPLY_DEX;
	paf->modifier  = - UMAX(2, level / 12);
	paf->bitvector = AFF_SLOW;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel yourself slowing d o w n...", victim);
	act("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_stone_skin, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		if (victim == ch)
			act_char("Your skin is already as hard as a rock.", ch);
		else {
			act("$N is already as hard as can be.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_RESISTS, sn);
	paf->level     = level;
	paf->duration  = (10 + level / 5);
	INT(paf->location) = DAM_BASH;
	paf->modifier  = level / 7;
	affect_to_char(victim, paf);

	INT(paf->location) = DAM_SLASH;
	affect_to_char(victim, paf);

	INT(paf->location) = DAM_PIERCE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	act_char("Your skin turns to stone.", victim);
}

SPELL_FUN(spell_summon, sn, level, ch, vo)
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
	||  (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_GHOST))
	||  !can_see_room(ch, victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
					    ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
						ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(ch->in_room->area->area_flags, AREA_CLOSED)
	||  IS_SET(victim->in_room->area->area_flags, AREA_CLOSED)
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
		CHAR_DATA *master = NULL;

		if (victim->pMobIndex->pShop != NULL
		||  IS_SET(victim->pMobIndex->act, ACT_AGGRESSIVE)
		||  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)
		||  IS_SET(victim->pMobIndex->act, ACT_IMMSUMMON)
		||  NPC(victim)->hunter)
			failed = TRUE;

                /*
                 * can't summon charmed creature if master
                 * has PLR_NOSUMMON or !in_PK, the same for mounts
                 */

                if (IS_AFFECTED(victim, AFF_CHARM)
                &&  victim->master != NULL)
                        master = victim->master;
                else if (victim->mount != NULL)
                        master = victim->mount;

                if (master != NULL
                &&  (!in_PK(ch, master) ||
                     (!IS_NPC(master) &&
                      IS_SET(PC(master)->plr_flags, PLR_NOSUMMON))))
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

	teleport_char(victim, ch, ch->in_room,
		      "$n disappears suddenly.",
		      "$N has summoned you!",
		      "$n arrives suddenly.");
}

SPELL_FUN(spell_teleport, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->in_room == NULL
	||  IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  (!IS_NPC(ch) && victim->fighting != NULL)
	||  (!IS_NPC(ch) && !in_PK(ch, victim))
	||  (victim != ch
	&&  (saves_spell(level - 5, victim, DAM_OTHER)))) {
		act_char("You failed.", ch);
		return;
	}

        /*
         * can't teleport charmed creature if master
         * !in_PK, the same for mounts
         */
        if (IS_NPC(victim)) {
                CHAR_DATA *master = NULL;

                if (IS_AFFECTED(victim, AFF_CHARM)
                &&  victim->master != NULL)
                        master = victim->master;
                else if (victim->mount != NULL)
                        master = victim->mount;

                if (master != NULL
                &&  (!in_PK(ch, master) ||
                     (!IS_NPC(master) &&
                      saves_spell(level - 5, master, DAM_OTHER)))) {
                        act_char("You failed.", ch);
                        return;
                }
        }

	teleport_char(victim, ch, get_random_room(victim, NULL),
		      "$n vanishes!",
		      "You have been teleported.",
		      "$n slowly fades into existence.");
}

SPELL_FUN(spell_bamf, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->in_room == NULL
	||  (victim != ch && saves_spell(level, victim, DAM_OTHER))
	||  IS_SET(victim->in_room->room_flags, ROOM_PEACE | ROOM_SAFE)) {
		act_char("You failed.", ch);
		return;
	}

	teleport_char(victim, ch,
		      get_random_room(victim, victim->in_room->area),
		      "$n vanishes!",
		      "You have been teleported.",
		      "$n slowly fades into existence.");
}

SPELL_FUN(spell_ventriloquate, sn, level, ch, vo)
{
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	target_name = one_argument(target_name, speaker, sizeof(speaker));

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_name(speaker, vch->name))
			continue;

		if (saves_spell(level, vch, DAM_MENTAL)) {
			act("Someone makes $t say '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		} else {
			act("$t says '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		}
	}
}

SPELL_FUN(spell_weaken, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn) || saves_spell(level, victim, DAM_OTHER))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = (4 + level / 12);
	INT(paf->location) = APPLY_STR;
	paf->modifier  = -1 * (2 + level / 12);
	paf->bitvector = AFF_WEAKEN;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel your strength slip away.", victim);
	act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
}

/*
 * Draconian spells.
 */
SPELL_FUN(spell_acid_breath, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
	act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = calc_spell_damage(ch, level, sn);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	if (saves_spell(level, victim, DAM_ACID)) {
		inflict_effect("acid", victim, level/2, dam/4);
		damage(ch,victim, dam/2, sn, DAM_F_SHOW);
	} else {
		inflict_effect("acid", victim, level, dam);
		damage(ch, victim, dam, sn, DAM_F_SHOW);
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
			inflict_effect("fire", vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_F_SHOW);
		} else {
			inflict_effect("fire", vch, level, dam);
			damage(ch, vch, dam, sn, DAM_F_SHOW);
		}
	} else { /* partial damage */
		if (saves_spell(level - 2, vch, DAM_FIRE)) {
			inflict_effect("fire", vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_F_SHOW);
		} else {
			inflict_effect("fire", vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_F_SHOW);
		}
	}

	return NULL;
}

SPELL_FUN(spell_fire_breath, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(10, ch->hit);
	int hp_dam  = number_range(hpch/9 + 1, hpch/5);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);

	act("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!",
	    ch, NULL, victim, TO_VICT);
	act("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

	inflict_effect("fire", victim->in_room, level, dam / 2);
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
			inflict_effect("cold", vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_F_SHOW);
		} else {
			inflict_effect("cold", vch, level, dam);
			damage(ch, vch, dam, sn, DAM_F_SHOW);
		}
	} else {
		if (saves_spell(level - 2, vch, DAM_COLD)) {
			inflict_effect("cold", vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_F_SHOW);
		} else {
			inflict_effect("cold", vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_F_SHOW);
		}
	}

	return NULL;
}

SPELL_FUN(spell_frost_breath, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(12, ch->hit);
	int hp_dam = number_range(hpch/11 + 1, hpch/6);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes out a freezing cone of frost!",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",
	    ch, NULL, victim, TO_VICT);
	act("You breath out a cone of frost.",
	    ch, NULL, NULL, TO_CHAR);

	inflict_effect("cold", victim->in_room, level, dam/2);
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
		inflict_effect("poison", vch, level/2, dam/4);
		damage(ch, vch, dam/2, sn, DAM_F_SHOW);
	} else {
		inflict_effect("poison", vch, level, dam);
		damage(ch, vch, dam, sn, DAM_F_SHOW);
	}

	return NULL;
}

SPELL_FUN(spell_gas_breath, sn, level, ch, vo)
{
	int hpch = UMAX(16, ch->hit);
	int hp_dam = number_range(hpch/15 + 1, 8);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes out a cloud of poisonous gas!",
	    ch, NULL, NULL, TO_ROOM);
	act("You breath out a cloud of poisonous gas.",
	    ch, NULL, NULL, TO_CHAR);
	inflict_effect("poison", ch->in_room, level, dam);
	vo_foreach(ch->in_room, &iter_char_room, gas_breath_cb,
		   sn, level, ch, dam);
}

SPELL_FUN(spell_lightning_breath, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(10, ch->hit);
	int hp_dam = number_range(hpch/9 + 1, hpch/5);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes a bolt of lightning at $N.",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!",
	    ch, NULL, victim, TO_VICT);
	act("You breathe a bolt of lightning at $N.",
	    ch, NULL, victim, TO_CHAR);

	if (saves_spell(level, victim, DAM_LIGHTNING)) {
		inflict_effect("shock", victim, level/2, dam/4);
		damage(ch, victim, dam/2, sn, DAM_F_SHOW);
	} else {
		inflict_effect("shock", victim, level, dam);
		damage(ch, victim, dam, sn, DAM_F_SHOW);
	}
}

SPELL_FUN(spell_find_object, sn, level, ch, vo)
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
			buffer = buf_new(0);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)) {
			buf_printf(buffer, BUF_END, "One is carried by %s\n",
			    PERS(in_obj->carried_by, ch, GET_LANG(ch), ACT_FORMSH));
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

SPELL_FUN(spell_lightning_shield, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already shielded.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected(ch,sn)) {
		act_char("This spell is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= ch->level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->level	= level;
	paf->duration	= level / 40;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with lightnings.", ch);
	act("The room starts to be filled with $n's lightnings.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_shocking_trap, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already trapped with shocks waves.",
		    ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("This spell is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= ch->level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 40;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with shock waves.", ch);
	act("The room starts to be filled with $n's shock waves.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_spectral_furor, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("The fabric of the cosmos strains in fury about $N!",
	    ch, NULL, victim, TO_NOTVICT);

	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_disruption, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A weird energy encompasses $N, causing you to question $S continued existence.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_sonic_resonance, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

/* acid */
SPELL_FUN(spell_sulfurus_spray, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A stinking spray of sulfurous liquid rains down on $N." ,
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_caustic_font, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A fountain of caustic liquid forms below $N. The smell of $S degenerating tissues is revolting!",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_acetum_primus, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

/*  Electrical  */
SPELL_FUN(spell_galvanic_whip, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$n conjures a whip of ionized particles, which lashes ferociously at $N.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_magnetic_thrust, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("An unseen energy moves nearby, causing your hair to stand on end!",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(spell_quantum_spike, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

/* negative */
SPELL_FUN(spell_hand_of_undead, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act_char("You feel a momentary chill.", victim);
		return;
	}

	if (IS_SET(victim->form, FORM_UNDEAD)) {
		 act_char("Your victim is unaffected by hand of undead.", ch);
		 return;
	}

	if (victim->level <= 2)
		dam		 = ch->hit + 1;
	else {
		dam = calc_spell_damage(ch, level, sn) * 2 / 3;

		victim->mana	/= 2;
		victim->move	/= 2;
		ch->hit		+= dam / 2;
	}

	act_char("You feel your life slipping away!", victim);
	act("$N is grasped by an incomprehensible hand of undead!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

static inline void
astral_walk(CHAR_DATA *ch, CHAR_DATA *victim)
{
	teleport_char(ch, victim, victim->in_room,
		      "$n disappears in a flash of light!",
		      "You travel via astral planes and go to $N.",
		      "$n appears in a flash of light!");
}

/* travel via astral plains */
SPELL_FUN(spell_astral_walk, sn, level, ch, vo)
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


/* travel via astral plains */
SPELL_FUN(spell_helical_flow, sn, level, ch, vo)
{
	CHAR_DATA *victim;


	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 3
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	teleport_char(ch, NULL, victim->in_room,
		      "$n coils into an ascending column of colour, vanishing into thin air.",
		      "You coil into an ascending column of colour, vanishing into thin air.",
		      "A coil of colours descends from above, revealing $n as it dissipates.");
}

SPELL_FUN(spell_corruption, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_CORRUPTION)) {
		act("$N is already corrupting.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_IMMORTAL(victim)
	||  saves_spell(level, victim, DAM_NEGATIVE)
	||  IS_SET(victim->form, FORM_UNDEAD)) {
		if (ch == victim) {
			act_puts("You feel momentarily ill, but it passes.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act_puts("$N seems to be unaffected.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level * 3 / 4;
	paf->duration	= 10 + level / 5;
	paf->bitvector	= AFF_CORRUPTION;
	affect_join(victim, paf);
	aff_free(paf);

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

	if (saves_spell(level, vch, DAM_AIR))
		damage(ch, vch, *pdam/2, sn, DAM_F_SHOW);
	else
		damage(ch, vch, *pdam, sn, DAM_F_SHOW);
	return NULL;
}

SPELL_FUN(spell_hurricane, sn, level, ch, vo)
{
	int hpch = UMAX(16, ch->hit);
	int hp_dam = number_range(hpch/15 + 1, 8);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n prays the gods of the storm for help.",
	    ch, NULL, NULL, TO_NOTVICT);
	act("You pray the gods of the storm to help you.",
	    ch, NULL, NULL, TO_CHAR);
	vo_foreach(ch->in_room, &iter_char_room, hurricane_cb,
		   sn, level, ch, &dam);
}

SPELL_FUN(spell_detect_undead, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_UNDEAD)) {
		if (victim == ch)
			act_char("You can already sense undead.", ch);
		else {
			act("$N can already detect undead.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_UNDEAD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(spell_take_revenge, sn, level, ch, vo)
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
		teleport_char(ch, NULL, room, NULL, NULL, NULL);
}

static ROOM_INDEX_DATA *
check_place(CHAR_DATA *ch, const char *argument)
{
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *dest_room;
	int number,door;
	int range = (ch->level / 10) + 1;
	char arg[MAX_INPUT_LENGTH];

	number = number_argument(argument, arg, sizeof(arg));
	if ((door = exit_lookup(arg)) == -1)
		return NULL;

	dest_room = ch->in_room;
	while (number > 0) {
		number--;
		if (--range < 1)
			return NULL;

		if ((pExit = dest_room->exit[door]) == NULL
		||  (dest_room = pExit->to_room.r) == NULL
		|| IS_SET(pExit->exit_info,EX_CLOSED))
			break;

		if (number < 1)
			return dest_room;
	}

	return NULL;
}

#define OBJ_VNUM_PORTAL			25

SPELL_FUN(spell_portal, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->level >= level + 3
	||  saves_spell(level, victim, DAM_NONE)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	stone = get_eq_char(ch, WEAR_HOLD);
	if (!IS_IMMORTAL(ch)
	&&  (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
		act_char("You lack the proper component for this spell.", ch);
		return;
	}

	if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
		act("It flares brightly and vanishes!",
		    ch, stone, NULL, TO_CHAR);
		extract_obj(stone, 0);
	}

	portal = create_obj(OBJ_VNUM_PORTAL, 0);
	if (portal == NULL)
		return;

	portal->timer = 2 + level / 25;
	INT(portal->value[3]) = victim->in_room->vnum;

	obj_to_room(portal, ch->in_room);

	act("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
	act("$p rises up before you.", ch, portal, NULL, TO_CHAR);
}

SPELL_FUN(spell_nexus, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;
	ROOM_INDEX_DATA *to_room, *from_room;

	from_room = ch->in_room;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->level >= level + 3
	||  !can_see_room(ch, from_room)
	||  saves_spell(level, victim, DAM_NONE)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	to_room = victim->in_room;

	stone = get_eq_char(ch,WEAR_HOLD);
	if (!IS_IMMORTAL(ch)
	&&  (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
		act_char("You lack the proper component for this spell.", ch);
		return;
	}

	if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
		act("It flares brightly and vanishes!",
		    ch, stone, NULL, TO_CHAR);
		extract_obj(stone, 0);
	}

	/* portal one */
	portal = create_obj(OBJ_VNUM_PORTAL, 0);
	if (portal == NULL)
		return;

	portal->timer = 1 + level / 10;
	INT(portal->value[3]) = to_room->vnum;

	obj_to_room(portal,from_room);

	act("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
	act("$p rises up before you.", ch, portal, NULL,TO_CHAR);

	/* no second portal if rooms are the same */
	if (to_room == from_room)
		return;

	/* portal two */
	portal = create_obj(OBJ_VNUM_PORTAL, 0);
	if (portal == NULL)
		return;
	portal->timer = 1 + level/10;
	INT(portal->value[3]) = from_room->vnum;

	obj_to_room(portal, to_room);

	if (to_room->people != NULL) {
		act("$p rises up from the ground.",
		    to_room->people, portal, NULL, TO_ROOM);
		act("$p rises up from the ground.",
		    to_room->people, portal, NULL, TO_CHAR);
	}
}

SPELL_FUN(spell_disintegrate, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *tmp_ch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int i,dam=0;
	OBJ_DATA *tattoo, *clanmark;
	PC_DATA *vpc;

	if (saves_spell(level + 9, victim, DAM_ENERGY)
	||  dice_wlb(1, 3, victim, NULL) == 1
	||  IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)
	||  IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		dam = calc_spell_damage(ch, level, sn);
		damage(ch, victim, dam, sn, DAM_F_SHOW);
		return;
	}

	act_puts("$N's thin light ray ###DISINTEGRATES### you!",
	      victim, NULL, ch, TO_CHAR, POS_RESTING);
	act_puts("$n's thin light ray ###DISINTEGRATES### $N!",
	      ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("Your thin light ray ###DISINTEGRATES### $N!",
	      ch, NULL, victim, TO_CHAR, POS_RESTING);
	act_char("You die..", victim);

	act("$N does not exist anymore!\n", ch, NULL, victim, TO_ROOM);

	act_char("You turn into an invincible ghost for a few minutes.", victim);
	act_char("As long as you don't attack anything.", victim);

	/*  disintegrate the objects... */
	tattoo = get_eq_char(victim, WEAR_TATTOO); /* keep tattoos for later */
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if ((clanmark = get_eq_char(victim, WEAR_CLANMARK)) != NULL)
		obj_from_char(clanmark);

	victim->gold = 0;
	victim->silver = 0;

	for (obj = victim->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, 0);
	}

	if (IS_NPC(victim)) {
		quest_handle_death(ch, victim);
		victim->pMobIndex->killed++;
		extract_char(victim, 0);
		return;
	}

	rating_update(ch, victim);
	extract_char(victim, XC_F_INCOMPLETE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by	= 0;
	victim->has_invis	= 0;
	victim->has_detect	= 0;
	for (i = 0; i < 4; i++)
		victim->armor[i]= 100;
	victim->position      = POS_RESTING;
	victim->hit           = 1;
	victim->mana	  = 1;

	vpc = PC(victim);
	REMOVE_BIT(vpc->plr_flags, PLR_BOUGHT_PET);
	SET_WANTED(victim, NULL);

	vpc->condition[COND_THIRST] = 40;
	vpc->condition[COND_HUNGER] = 40;
	vpc->condition[COND_FULL] = 40;
	vpc->condition[COND_BLOODLUST] = 40;
	vpc->condition[COND_DESIRE] = 40;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	for (tmp_ch = npc_list; tmp_ch; tmp_ch = tmp_ch->next)
		if (NPC(tmp_ch)->last_fought == victim)
			NPC(tmp_ch)->last_fought = NULL;
}

SPELL_FUN(spell_bark_skin, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		if (victim == ch)
			act_char("Your skin is already covered in bark.", ch);
		else {
			act("$N is already as hard as can be.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level;
	INT(paf->location) = APPLY_AC;
	paf->modifier  = -(level * 1.5);
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n's skin becomes covered in bark.", victim, NULL, NULL, TO_ROOM);
	act_char("Your skin becomes covered in bark.", victim);
}

#define OBJ_VNUM_RANGER_STAFF		28

SPELL_FUN(spell_ranger_staff, sn, level, ch, vo)
{
	OBJ_DATA *staff;
	AFFECT_DATA *paf;

	staff = create_obj(OBJ_VNUM_RANGER_STAFF, 0);
	if (staff == NULL)
		return;

	staff->level = ch->level;
	INT(staff->value[1]) = 5 + level / 14;
	INT(staff->value[2]) = 4 + level / 15;

	act("You create $p!", ch, staff, NULL, TO_CHAR);
	act("$n creates $p!", ch, staff, NULL, TO_ROOM);

	paf = aff_new(TO_OBJECT, sn);
	paf->level              = ch->level;
	paf->duration           = -1;
	INT(paf->location)	= APPLY_HITROLL;
	paf->modifier           = 2 + level/5;
	affect_to_obj(staff, paf);

	INT(paf->location)	= APPLY_DAMROLL;
	affect_to_obj(staff, paf);
	aff_free(paf);

	staff->timer = level;
	staff->level = ch->level;

	obj_to_char(staff, ch);
}

SPELL_FUN(spell_transform, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn) || ch->hit > ch->max_hit) {
		act_char("You are already overflowing with health.", ch);
		return;
	}

	ch->hit += UMIN(30000 - ch->max_hit, ch->max_hit);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= UMIN(30000 - ch->max_hit, ch->max_hit);
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Your mind clouds as your health increases.", ch);
}

SPELL_FUN(spell_mana_transfer, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim == ch) {
		act_char("You would implode if you tried to transfer mana to yourself.", ch);
		return;
	}

	if (!IS_CLAN(ch->clan, victim->clan)) {
		act_char("You may only cast this spell on your clannies.", ch);
		return;
	}

	if (ch->hit > 50)
		victim->mana = UMIN(victim->max_mana, victim->mana + number_range(20,120));
	damage(ch, ch, 50, sn, DAM_F_SHOW);
}

SPELL_FUN(spell_mental_knife, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (ch->level < MAX_LEVEL / 2)
		dam = dice(level, 8);
	else if (ch->level < MAX_LEVEL * 2 / 3)
		dam = dice(level, 11);
	else
		dam = dice(level, 14);

	if (saves_spell(level, victim, DAM_MENTAL))
	      dam /= 2;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_sn_affected(victim, sn)
	&&  !saves_spell(level, victim, DAM_MENTAL)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= level;
		INT(paf->location) = APPLY_INT;
		paf->modifier	= -7;
		affect_to_char(victim, paf);

		INT(paf->location) = APPLY_WIS;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("Your mental knife sears $N's mind!",
		    ch, NULL, victim, TO_CHAR);
		act("$n's mental knife sears your mind!",
		    ch, NULL, victim, TO_VICT);
		act("$n's mental knife sears $N's mind!",
		    ch, NULL, victim, TO_NOTVICT);
	}
}

#define MOB_VNUM_DEMON			13

SPELL_FUN(spell_demon_summon, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to summon another demon right now.", ch);
		return;
	}

	act_char("You attempt to summon a demon.", ch);
	act("$n attempts to summon a demon.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_DEMON) {
			act_char("Two demons are more than you can control!", ch);
			return;
		}
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level              = level;
	paf->duration           = 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("A demon arrives from underworld!", ch);
		act("A demon arrives from the underworld!",
		    ch, NULL, NULL, TO_ROOM);
		act_char("But suddenly returns back.", ch);
		act("But suddenly returns back.", ch, NULL, NULL, TO_ROOM);
		return;
	}

	demon = create_mob(MOB_VNUM_DEMON, 0);
	if (demon == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		demon->perm_stat[i] = ch->perm_stat[i];

	SET_HIT(demon, URANGE(ch->perm_hit, ch->hit, 30000));
	SET_MANA(demon, ch->perm_mana);
	demon->level = level;
	for (i = 0; i < 3; i++)
		demon->armor[i] = interpolate(demon->level,100,-100);
	demon->armor[3] = interpolate(demon->level,100,0);
	demon->gold = 0;
	demon->silver = 0;
	NPC(demon)->dam.dice_number = number_range(level/15, level/10);
	NPC(demon)->dam.dice_type = number_range(level/3, level/2);
	demon->damroll = number_range(level/8, level/6);

	act("A demon arrives from the underworld!", ch, NULL, NULL, TO_ROOM);

	char_to_room(demon, ch->in_room);
	if (IS_EXTRACTED(demon))
		return;

	if (number_percent() < 40) {
		if (can_see(demon, ch))
			dofun("say", demon, "You dare disturb me\?\?!!!");
		else
			dofun("say", demon, "Who dares disturb me\?\?!!!");
		multi_hit(demon, ch, NULL);
	} else {
		SET_BIT(demon->affected_by, AFF_CHARM);
		demon->master = demon->leader = ch;
	}
}

static void *
scourge_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;

	if (number_percent() < level * 2)
		spellfun_call("poison", NULL, level, ch, vch);

	if (number_percent() < level * 2)
		spellfun_call("blindness", NULL, level, ch, vch);

	if (number_percent() < level * 2)
		spellfun_call("weaken", NULL, level, ch, vch);

	if (saves_spell(level, vch, DAM_FIRE))
		dam /= 2;
	damage(ch, vch, dam, sn, DAM_F_SHOW);
	return NULL;
}

SPELL_FUN(spell_scourge, sn, level, ch, vo)
{
/*	dam = calc_spell_damage(ch, level, sn); */
	int dam = ch->level < MAX_LEVEL / 2 ? dice(level, 6) :
		  ch->level < MAX_LEVEL * 2 / 3 ? dice(level, 9) :
				   dice(level, 12);
	vo_foreach(ch->in_room, &iter_char_room, scourge_cb,
		   sn, level, ch, dam);
}

SPELL_FUN(spell_manacles, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!IS_WANTED(victim)) {
		act("But $N is not wanted.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_sn_affected(victim, sn)
	||  saves_spell(ch->level, victim, DAM_CHARM))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5 + level/5;
	paf->modifier	= -get_curr_stat(victim, STAT_DEX) + 4;
	INT(paf->location)= APPLY_DEX;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -5;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DAMROLL;
	paf->modifier	= -10;
	affect_to_char(victim, paf);
	aff_free(paf);

	spellfun_call("charm person", sn, level, ch, vo);
}

#define OBJ_VNUM_RULER_SHIELD1		71
#define OBJ_VNUM_RULER_SHIELD2		72
#define OBJ_VNUM_RULER_SHIELD3		73
#define OBJ_VNUM_RULER_SHIELD4		74

SPELL_FUN(spell_shield_of_ruler, sn, level, ch, vo)
{
	int shield_vnum;
	OBJ_DATA *shield;
	AFFECT_DATA *paf;

	if (level >= 71)
		shield_vnum = OBJ_VNUM_RULER_SHIELD4;
	else if (level >= 51)
		shield_vnum = OBJ_VNUM_RULER_SHIELD3;
	else if (level >= 31)
		shield_vnum = OBJ_VNUM_RULER_SHIELD2;
	else
		shield_vnum = OBJ_VNUM_RULER_SHIELD1;

	shield = create_obj(shield_vnum, 0);
	if (shield == NULL)
		return;

	shield->level = ch->level;
	shield->timer = level;
	shield->cost  = 0;
	obj_to_char(shield, ch);

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->modifier	= level / 8;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(shield, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(shield, paf);

	paf->modifier	= -level/2;
	INT(paf->location)= APPLY_AC;
	affect_to_obj(shield, paf);

	paf->modifier	= UMAX(1, level / 30);
	INT(paf->location)= APPLY_CHA;
	affect_to_obj(shield, paf);
	aff_free(paf);

	act("You create $p!", ch, shield, NULL, TO_CHAR);
	act("$n creates $p!", ch, shield, NULL, TO_ROOM);
}

#define MOB_VNUM_SPECIAL_GUARD		11

SPELL_FUN(spell_guard_call, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *guard;
	CHAR_DATA *guard2;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to call another two guards now.", ch);
		return;
	}

	dofun("yell", ch, "Guards! Guards!");

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL &&
	     ch->in_room->exit[1] == NULL &&
	     ch->in_room->exit[2] == NULL &&
	     ch->in_room->exit[3] == NULL &&
	     ch->in_room->exit[4] == NULL &&
	     ch->in_room->exit[5] == NULL)) {
		act_char("No guard come to help you.", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SPECIAL_GUARD) {
			dofun("say", gch, "What? I'm not good enough?");
			return;
		}
	}

	guard = create_mob(MOB_VNUM_SPECIAL_GUARD, 0);
	if (guard == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		guard->perm_stat[i] = ch->perm_stat[i];

	guard->max_hit = 2*ch->max_hit;
	guard->hit = guard->max_hit;
	guard->max_mana = ch->max_mana;
	guard->mana = guard->max_mana;
	guard->alignment = ch->alignment;
	guard->level = ch->level;
	for (i=0; i < 3; i++)
		guard->armor[i] = interpolate(guard->level,100,-200);
	guard->armor[3] = interpolate(guard->level,100,-100);
	guard->gold = 0;
	guard->silver = 0;

	NPC(guard)->dam.dice_number = number_range(level/18, level/14);
	NPC(guard)->dam.dice_type = number_range(level/4, level/3);
	guard->damroll = number_range(level/10, level/8);

	guard2 = clone_mob(guard);

	SET_BIT(guard->affected_by, AFF_CHARM);
	SET_BIT(guard2->affected_by, AFF_CHARM);
	guard->master = guard2->master = ch;
	guard->leader = guard2->leader = ch;

	act_char("Two guards come to your rescue!", ch);
	act("Two guards come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 12;
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(guard,ch->in_room);
	char_to_room(guard2,ch->in_room);
}

#define MOB_VNUM_NIGHTWALKER		14

SPELL_FUN(spell_nightwalker, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *walker;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_puts("You feel too weak to summon a Nightwalker now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act_puts("You attempt to summon a Nightwalker.",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	act("$n attempts to summon a Nightwalker.", ch, NULL, NULL, TO_ROOM);

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("You notice some motions in shadows around you.", ch);
		act_char("But nothing else happens.", ch);
		act("But noone appears.", ch, NULL, NULL, TO_ROOM);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_NIGHTWALKER) {
			act_puts("Two Nightwalkers are more than "
				 "you can control!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("But noone appears.", ch, NULL, NULL, TO_ROOM);
			return;
		}
	}

	walker = create_mob(MOB_VNUM_NIGHTWALKER, 0);
	if (walker == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		walker->perm_stat[i] = ch->perm_stat[i];

	SET_HIT(walker, ch->perm_hit);
	SET_MANA(walker, ch->max_mana);
	walker->level = level;
	for (i = 0; i < 3; i++)
		walker->armor[i] = interpolate(walker->level, 100, -100);
	walker->armor[3] = interpolate(walker->level, 100, 0);
	walker->gold = 0;
	walker->silver = 0;
	NPC(walker)->dam.dice_number = number_range(level/15, level/10);
	NPC(walker)->dam.dice_type   = number_range(level/3, level/2);
	walker->damroll  = 0;

	act_puts("$N rises from the shadows!",
		 ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N rises from the shadows!", ch, NULL, walker, TO_ROOM);
	act_puts("$N kneels before you.",
		 ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N kneels before $n!", ch, NULL, walker, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	walker->master = walker->leader = ch;

	char_to_room(walker, ch->in_room);
}

SPELL_FUN(spell_eyes_of_intrigue, sn, level, ch, vo)
{
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, target_name)) == NULL) {
		act_char("Your spy network reveals no such player.", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_NONE)) {
		act_char("Your spy network cannot find that player.", ch);
		return;
	}

	if (ch->in_room == victim->in_room)
		dofun("look", ch, str_empty);
	else
		look_at(ch, victim->in_room);
}

SPELL_FUN(spell_shadow_cloak, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!IS_CLAN(ch->clan, victim->clan)) {
		act_char("You may only cast this spell on your clannies.", ch);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		if (victim == ch) {
			act_char("You are already protected by a shadow cloak.",
				 ch);
		} else {
			act("$N is already protected by a shadow cloak.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	paf->modifier	= -level;
	INT(paf->location)= APPLY_AC;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel the shadows protect you.", victim);
	if (ch != victim) {
		act("A cloak of shadows protect $N.",
		    ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(spell_nightfall, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	OBJ_DATA  *obj;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You can't find the power to control objects.", ch);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_same_group(ch, vch))
			continue;

		for (obj = vch->carrying; obj; obj = obj->next_content) {
			if (obj->item_type != ITEM_LIGHT
			||  INT(obj->value[2]) == 0
			||  saves_spell(level, vch, DAM_ENERGY))
				continue;

			act("$p flickers and goes out!", ch, obj, NULL, TO_ALL);
			INT(obj->value[2]) = 0;

			if (obj->wear_loc == WEAR_LIGHT
			&&  ch->in_room->light > 0)
				ch->in_room->light--;
		}
	}

	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->item_type != ITEM_LIGHT
		||  INT(obj->value[2]) == 0)
			continue;

		act("$p flickers and goes out!", ch, obj, NULL, TO_ALL);
		INT(obj->value[2]) = 0;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= 2;
	affect_to_char(ch, paf);
	aff_free(paf);
}

SPELL_FUN(spell_garble, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (ch == victim) {
		act_char("Garble whose speech?", ch);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		act("$N's speech is already garbled.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe_nomessage(ch, victim)) {
		act_char("You cannot garble that person.", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = 10;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("You have garbled $N's speech!",ch,NULL,victim,TO_CHAR);
	act_char("You feel your tongue contort.", victim);
}

SPELL_FUN(spell_confuse, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	CHAR_DATA *rch;
	int count;

	if (is_sn_affected(victim, "confuse")) {
		act("$N is already thoroughly confused.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 10;
	affect_to_char(victim, paf);
	aff_free(paf);

	count = 0;
	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != ch
		&&  can_see(ch, rch)
		&&  !is_safe(ch, rch))
			count++;
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != ch
		&&  can_see(ch, rch)
		&&  !is_safe(ch, rch)
		&&  number_range(1, count) == 1)
			break;
	}

	if (rch)
		multi_hit(victim, rch, NULL);
	else
		multi_hit(victim, ch, NULL);
}

SPELL_FUN(spell_terangreal, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_NPC(victim))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 10;
	paf->bitvector	= AFF_SLEEP;
	affect_join(victim, paf);
	aff_free(paf);

	if (IS_AWAKE(victim)) {
		act_char("You are overcome by a sudden surge of fatigue.",
			 victim);
		act("$n falls into a deep sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
}

SPELL_FUN(spell_kassandra, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("The kassandra has been used for this purpose too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = 5;
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->hit = UMIN(ch->hit + 150, ch->max_hit);
	update_pos(ch);
	act_char("A warm feeling fills your body.", ch);
	act("$n looks better.", ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_sebat, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("The kassandra has been used for that too recently.",
			 ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level;
	INT(paf->location) = APPLY_AC;
	paf->modifier	= -30;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n is surrounded by a mystical shield.",ch, NULL,NULL,TO_ROOM);
	act_char("You are surrounded by a mystical shield.", ch);
}

SPELL_FUN(spell_matandra, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("The kassandra has been used for this purpose too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = 5;
	affect_to_char(ch, paf);
	aff_free(paf);

	dam = calc_spell_damage(ch, level, sn);

	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(spell_amnesia, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	pc_skill_t *pc_sk;

	if (IS_NPC(victim))
		return;

	C_FOREACH(pc_sk, &PC(victim)->learned)
		pc_sk->percent = UMAX(pc_sk->percent / 2, 1);

	act("You feel your memories slip away.", victim, NULL, NULL, TO_CHAR);
	act("$n gets a blank look on $s face.", victim, NULL, NULL, TO_ROOM);
}

#define OBJ_VNUM_CHAOS_BLADE		97

SPELL_FUN(spell_chaos_blade, sn, level, ch, vo)
{
	OBJ_DATA *blade;
	AFFECT_DATA *paf;

	blade = create_obj(OBJ_VNUM_CHAOS_BLADE, 0);
	if (blade == NULL)
		return;

	blade->level = level;
	blade->timer = level * 2;
	INT(blade->value[2]) = (level / 10) + 3;

	act("You create $p!", ch, blade, NULL, TO_CHAR);
	act("$n creates $p!", ch, blade, NULL, TO_ROOM);

	paf = aff_new(TO_OBJECT, sn);
	paf->level        = level;
	paf->duration     = -1;
	paf->modifier     = level / 6;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(blade, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(blade, paf);
	aff_free(paf);

	obj_to_char(blade, ch);
}

SPELL_FUN(spell_stalker, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *stalker;
	AFFECT_DATA *paf;
	int i;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim == ch
	||  !IS_WANTED(victim)) {
		act_char("You failed.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("This power is used too recently.", ch);
		return;
	}

	act_char("You attempt to summon a stalker.", ch);
	act("$n attempts to summon a stalker.",ch,NULL,NULL,TO_ROOM);

	if (IS_SET(victim->in_room->room_flags,
		   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		/* This message was stolen from ADOM */
		act_char("You sense imminent danger...", victim);
		act_char("...but the feeling passes.", victim);
		return;
	}

	stalker = create_mob(MOB_VNUM_STALKER, 0);
	if (stalker == NULL)
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 6;
	affect_to_char(ch, paf);
	aff_free(paf);

	for (i = 0; i < MAX_STAT; i++)
		stalker->perm_stat[i] = victim->perm_stat[i];

	stalker->max_hit = UMIN(30000, 2 * victim->max_hit);
	stalker->hit = stalker->max_hit;
	stalker->max_mana = victim->max_mana;
	stalker->mana = stalker->max_mana;
	stalker->level = victim->level;

	NPC(stalker)->dam.dice_number =
	    number_range(victim->level/18, victim->level/14);
	NPC(stalker)->dam.dice_type =
	    number_range(victim->level/4, victim->level/3);
	stalker->damroll = number_range(victim->level/10, victim->level/8);

	for (i = 0; i < 3; i++)
		stalker->armor[i] = interpolate(stalker->level, 100, -100);
	stalker->armor[3] = interpolate(stalker->level, 100, 0);
	stalker->gold = 0;
	stalker->silver = 0;
	stalker->affected_by |= (ID_ALL_INVIS | ID_EVIL | ID_MAGIC | ID_GOOD);

	NPC(stalker)->target = victim;
	free_string(stalker->clan);
	stalker->clan   = str_qdup(ch->clan);
	act_char("An invisible stalker arrives to stalk you!", victim);
	act("An invisible stalker arrives to stalk $n!",victim,NULL,NULL,TO_ROOM);
	act_char("An invisible stalker has been sent.", ch);

	char_to_room(stalker,victim->in_room);
}

static inline void
tesseract_other(CHAR_DATA *ch, CHAR_DATA *victim, ROOM_INDEX_DATA *to_room)
{
	teleport_char(victim, ch, to_room,
		      NULL,
		      "$N utters some strange words and, "
		      "with a sickening lurch, you feel time\n"
		      "and space shift around you.",
		      "$n arrives suddenly.");
}

SPELL_FUN(spell_tesseract, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	CHAR_DATA *wch;
	CHAR_DATA *wch_next;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	pet = GET_PET(ch);
	if (pet && pet->in_room != ch->in_room)
		pet = NULL;

	for (wch = ch->in_room->people; wch; wch = wch_next) {
		wch_next = wch->next_in_room;
		if (wch != ch && wch != pet && is_same_group(wch, ch))
			tesseract_other(ch, wch, victim->in_room);
	}

	act("With a sudden flash of light, $n and $s friends disappear!",
		ch, NULL, NULL, TO_ROOM);
	teleport_char(ch, NULL, victim->in_room,
		      NULL,
		      "As you utter the words, time and space seem to blur.\n"
		      "You feel as though space and time are shifting\n"
		      "all around you while you remain motionless.",
		      "$n arrives suddenly.");

	if (pet)
		tesseract_other(ch, pet, victim->in_room);
}

#define OBJ_VNUM_POTION_SILVER		43
#define OBJ_VNUM_POTION_GOLDEN		44
#define OBJ_VNUM_POTION_SWIRLING	45

SPELL_FUN(spell_brew, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *potion;
	OBJ_DATA *vial;
	const char *spell;

	if (obj->item_type != ITEM_TRASH
	&&  obj->item_type != ITEM_TREASURE
	&&  obj->item_type != ITEM_KEY) {
		act_char("That can't be transformed into a potion.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be brewed.", ch);
		return;
	}

	for (vial = ch->carrying; vial != NULL; vial = vial->next_content)
		if (vial->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL)
			break;

	if (vial == NULL)  {
		act_char("You don't have any vials to brew the potion into.", ch);
		return;
	}

	if (number_percent() < 50) {
		act_char("You failed and destroyed it.", ch);
		extract_obj(obj, 0);
		return;
	}

	if (obj->item_type == ITEM_TRASH)
		potion = create_obj(OBJ_VNUM_POTION_SILVER, 0);
	else if (obj->item_type == ITEM_TREASURE)
		potion = create_obj(OBJ_VNUM_POTION_GOLDEN, 0);
	else
		potion = create_obj(OBJ_VNUM_POTION_SWIRLING, 0);
	if (potion == NULL)
		return;

	potion->label = str_qdup(vial->label);
	potion->level = ch->level;
	INT(potion->value[0]) = level;

	spell = 0;

	switch (obj->item_type) {
	case ITEM_TRASH:
		switch(number_bits(3)) {
		case 0:
			spell = "fireball";
			break;
		case 1:
			spell = "cure poison";
			break;
		case 2:
			spell = "cure blindness";
			break;
		case 3:
			spell = "cure disease";
			break;
		case 4:
			spell = "word of recall";
			break;
		case 5:
			spell = "protection good";
			break;
		case 6:
			spell = "protection evil";
			break;
		case 7:
			spell = "sanctuary";
			break;
		}
		break;

	case ITEM_TREASURE:
		switch(number_bits(3)) {
		case 0:
			spell = "cure critical wounds";
			break;
		case 1:
			spell = "haste";
			break;
		case 2:
			spell = "frenzy";
			break;
		case 3:
			spell = "create spring";
			break;
		case 4:
			spell = "holy word";
			break;
		case 5:
			spell = "invisibility";
			break;
		case 6:
			spell = "cure light wounds";
			break;
		case 7:
			spell = "cure serious wounds";
			break;
		}
		break;

	case ITEM_KEY:
		switch (number_bits(3)) {
		case 0:
			spell = "detect magic";
			break;
		case 1:
			spell = "detect invis";
			break;
		case 2:
			spell = "pass door";
			break;
		case 3:
			spell = "detect hidden";
			break;
		case 4:
			spell = "improved detect";
			break;
		case 5:
			spell = "acute vision";
			break;
		case 6:
			spell = "detect good";
			break;
		case 7:
			spell = "detect evil";
			break;
		}
		break;
	}

	STR_ASSIGN(potion->value[1], str_dup(spell));
	extract_obj(obj, 0);
	act("You brew $p from your resources!", ch, potion, NULL, TO_CHAR);
	act("$n brews $p from $s resources!", ch, potion, NULL, TO_ROOM);
	obj_to_char(potion, ch);

	extract_obj(vial, 0);
}

SPELL_FUN(spell_shadowlife, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *shadow;
	AFFECT_DATA *paf;
	int i;

	if (IS_NPC(victim)) {
		act_char("Now, why would you want to do that?!?", ch);
		return;
	}

	if (is_sn_affected(ch,sn)) {
		act_char("You don't have the strength to raise a Shadow now.", ch);
		return;
	}
	if (IS_SET(victim->in_room->room_flags,
		   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("You notice something unusual about your shadow.",
		    victim);
		return;
	}

	act("You give life to $N's shadow!", ch, NULL, victim, TO_CHAR);
	act("$n gives life to $N's shadow!", ch, NULL, victim, TO_NOTVICT);
	act("$n gives life to your shadow!", ch, NULL, victim, TO_VICT);

	shadow = create_mob_of(MOB_VNUM_SHADOW, &victim->short_descr);
	if (shadow == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		shadow->perm_stat[i] = ch->perm_stat[i];

	shadow->max_hit = (3 * ch->max_hit) / 4;
	shadow->hit = shadow->max_hit;
	shadow->max_mana = (3 * ch->max_mana) / 4;
	shadow->mana = shadow->max_mana;
	shadow->alignment = ch->alignment;
	shadow->level = level;
	for (i = 0; i < 3; i++)
		shadow->armor[i] = interpolate(shadow->level,100,-100);
	shadow->armor[3] = interpolate(shadow->level,100,0);
	shadow->gold = 0;
	shadow->silver = 0;

	NPC(shadow)->target = victim;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level        = level;
	paf->duration     = 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(shadow, ch->in_room);
	multi_hit(shadow, victim, NULL);
}

#define OBJ_VNUM_RULER_BADGE		70

SPELL_FUN(spell_ruler_badge, sn, level, ch, vo)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;

	if (get_eq_char(ch, WEAR_NECK) != NULL) {
		act_puts("But you are wearing something else on your neck.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	for (badge = ch->carrying; badge != NULL; badge = obj_next) {
		obj_next = badge->next_content;

		if (badge->pObjIndex->vnum == OBJ_VNUM_RULER_BADGE) {
			act("Your $p vanishes.", ch, badge, NULL, TO_CHAR);
			extract_obj(badge, 0);
		}
	}

	badge = create_obj(OBJ_VNUM_RULER_BADGE, 0);
	if (badge == NULL)
		return;

	badge->level = ch->level;

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->modifier	= level;

	INT(paf->location)= APPLY_HIT;
	affect_to_obj(badge, paf);

	INT(paf->location)= APPLY_MANA;
	affect_to_obj(badge, paf);

	paf->modifier	= level / 8;
	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(badge, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(badge, paf);
	aff_free(paf);

	badge->timer = 200;
	act("You wear $p!",ch, NULL, NULL, TO_CHAR);
	act("$n wears $s $p!", ch, NULL, NULL, TO_ROOM);

	obj_to_char(badge, victim);
	equip_char(ch, badge, WEAR_NECK);
}

SPELL_FUN(spell_remove_badge, sn, level, ch, vo)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_next;

	for (badge = victim->carrying; badge != NULL; badge = obj_next) {
		obj_next = badge->next_content;
		if (badge->pObjIndex->vnum == OBJ_VNUM_RULER_BADGE) {
			act("Your $p vanishes.",ch, badge, NULL, TO_CHAR);
			act("$n's $p vanishes.", ch, badge, NULL, TO_ROOM);
			extract_obj(badge, 0);
			continue;
		}
	}
}

SPELL_FUN(spell_dragon_strength, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are already full of the strength of the dragon.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 3;
	paf->modifier = 2;

	INT(paf->location) = APPLY_HITROLL;
	affect_to_char(ch, paf);

	INT(paf->location) = APPLY_DAMROLL;
	affect_to_char(ch, paf);

	paf->modifier = 10;
	INT(paf->location) = APPLY_AC;
	affect_to_char(ch, paf);

	paf->modifier = 2;
	INT(paf->location) = APPLY_STR;
	affect_to_char(ch, paf);

	paf->modifier = -2;
	INT(paf->location) = APPLY_DEX;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("The strength of the dragon enters you.", ch);
	act("$n looks a bit meaner now.", ch, NULL, NULL, TO_ROOM);
}
SPELL_FUN(spell_blue_dragon, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("blue dragon");

	act("$n turns $self into blue dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into blue dragon.", ch, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10 + 8;
	paf->location.s	= str_dup("blue dragon");
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_dup("enhanced damage");
	paf->modifier	= 10 + level/20;
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_AFFECTS;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= ch->max_hit / 3;
	paf->bitvector	= AFF_FLYING | AFF_TURNED;
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->hit += ch->max_hit/3;
	update_pos(ch);
}

SPELL_FUN(spell_green_dragon, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("green dragon");

	act("$n turns $self into green dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into green dragon.", ch, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10+8;
	paf->location.s	= str_dup("green dragon");
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_dup("enhanced damage");
	paf->modifier	= 7 + level/20;
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_AFFECTS;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= ch->max_hit/5;
	paf->bitvector	= AFF_FLYING | AFF_TURNED;
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->hit += ch->max_hit/5;
}

SPELL_FUN(spell_white_dragon, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("white dragon");

	act("$n turns $self into white dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into white dragon.", ch, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level / 10 + 8;
	paf->location.s	= str_dup("white dragon");
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_dup("enhanced damage");
	paf->modifier	= 7 + level / 20;
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_AFFECTS;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= ch->max_hit / 4;
	paf->bitvector	= AFF_FLYING | AFF_TURNED;
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->hit += ch->max_hit/4;
}

SPELL_FUN(spell_black_dragon, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("black dragon");

	act("$n turns $self into black dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into black dragon.", ch, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10 + 8;
	paf->location.s	= str_dup("black dragon");
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_dup("enhanced damage");
	paf->modifier	= 7 + level/20;
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_AFFECTS;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= ch->max_hit/4;
	paf->bitvector	= AFF_FLYING|AFF_TURNED;
	affect_to_char(ch, paf);

	ch->hit += ch->max_hit/4;
}

SPELL_FUN(spell_red_dragon, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_TURNED)) {
		act("Return to your natural form first.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("red dragon");

	act("$n turns $self into red dragon.", ch, NULL, NULL, TO_ROOM);
	act("You turn yourself into red dragon.", ch, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10+8;
	paf->location.s	= str_dup("red dragon");
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_dup("enhanced damage");
	paf->modifier	= 12 + level/20;
	paf->bitvector	= 0;
	affect_to_char(ch, paf);
	free_string(paf->location.s);

	paf->where	= TO_AFFECTS;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= ch->max_hit/2;
	affect_to_char(ch, paf);

	INT(paf->location)= APPLY_DAMROLL;
	paf->modifier	= 5 + level/10;
	paf->bitvector	= AFF_FLYING | AFF_TURNED;
	affect_to_char(ch, paf);

	ch->hit += ch->max_hit/2;
}

#define OBJ_VNUM_PLATE			82

SPELL_FUN(spell_dragon_plate, sn, level, ch, vo)
{
	OBJ_DATA *plate;
	AFFECT_DATA *paf;

	plate = create_obj(OBJ_VNUM_PLATE, 0);
	if (plate == NULL)
		return;

	plate->level = ch->level;
	plate->timer = 2 * level;
	plate->cost  = 0;
	plate->level  = ch->level;

	paf = aff_new(TO_OBJECT, sn);
	paf->level        = level;
	paf->duration     = -1;
	paf->modifier     = level / 8;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(plate, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(plate, paf);
	aff_free(paf);

	obj_to_char(plate, ch);

	act("You create $p!", ch, plate, NULL, TO_CHAR);
	act("$n creates $p!", ch, plate, NULL, TO_ROOM);
}

#define MOB_VNUM_SQUIRE			16

SPELL_FUN(spell_squire, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *squire;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_char("You cannot command another squire right now.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
	        act_char("You can't summon a squire here.", ch);
		return;
	}

	act_char("You attempt to summon a squire.", ch);
	act("$n attempts to summon a squire.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SQUIRE) {
			act_char("Two squires are more than you need!", ch);
			return;
		}
	}

	squire = create_mob(MOB_VNUM_SQUIRE, 0);
	if (squire == NULL)
		return;

	for (i=0;i < MAX_STAT; i++)
		squire->perm_stat[i] = ch->perm_stat[i];

	squire->max_hit = ch->max_hit;
	squire->hit = squire->max_hit;
	squire->max_mana = ch->max_mana;
	squire->mana = squire->max_mana;
	squire->level = ch->level;
	for (i=0; i < 3; i++)
	squire->armor[i] = interpolate(squire->level,100,-100);
	squire->armor[3] = interpolate(squire->level,100,0);
	squire->gold = 0;
	squire->silver = 0;

	NPC(squire)->dam.dice_number = number_range(level/20, level/15);
	NPC(squire)->dam.dice_type = number_range(level/4, level/3);
	squire->damroll = number_range(level/10, level/8);

	act_char("A squire arrives from nowhere!", ch);
	act("A squire arrives from nowhere!", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->type		= sn;
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	squire->master = squire->leader = ch;
	char_to_room(squire, ch->in_room);
}

#define OBJ_VNUM_DRAGONDAGGER		80
#define OBJ_VNUM_DRAGONMACE		81
#define OBJ_VNUM_DRAGONSWORD		83
#define OBJ_VNUM_DRAGONLANCE		99

SPELL_FUN(spell_dragon_weapon, sn, level, ch, vo)
{
	int sword_vnum = 0;
	OBJ_DATA *sword;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;

	target_name = one_argument(target_name, arg, sizeof(arg));
	if (!str_cmp(arg, "sword"))
		sword_vnum = OBJ_VNUM_DRAGONSWORD;
	else if (!str_cmp(arg, "mace"))
		sword_vnum = OBJ_VNUM_DRAGONMACE;
	else if (!str_cmp(arg, "dagger"))
		sword_vnum = OBJ_VNUM_DRAGONDAGGER;
	else if (!str_cmp(arg, "lance"))
		sword_vnum = OBJ_VNUM_DRAGONLANCE;
	else {
		act_puts("You can't make a Dragon Weapon like that!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	sword = create_obj(sword_vnum, 0);
	if (sword == NULL)
		return;

	sword->level = ch->level;
	sword->timer = level * 2;
	sword->cost  = 0;
	if (ch->level  < 50)
		INT(sword->value[2]) = (ch->level / 10);
	else
		INT(sword->value[2]) = (ch->level / 6) - 3;
	sword->level = ch->level;

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->modifier	= level / 5;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(sword, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(sword, paf);
	aff_free(paf);

	if (IS_GOOD(ch))
		SET_OBJ_STAT(sword, ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL);
	else if (IS_NEUTRAL(ch))
		SET_OBJ_STAT(sword, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
	else if (IS_EVIL(ch))
		SET_OBJ_STAT(sword, ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD);
	obj_to_char(sword, ch);

	act("You create $p!", ch, sword, NULL, TO_CHAR);
	act("$n creates $p!", ch, sword, NULL, TO_ROOM);
}

SPELL_FUN(spell_entangle, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (ch->in_room->sector_type == SECT_INSIDE
	||  ch->in_room->sector_type == SECT_CITY
	||  ch->in_room->sector_type == SECT_DESERT
	||  ch->in_room->sector_type == SECT_AIR) {
		act_char("No plants can grow here.", ch);
		return;
	}

	dam = number_range(level, 4 * level);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_F_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	act("The thorny plants spring up around $n, entangling $s legs!",
	    victim, NULL, NULL, TO_ROOM);
	act("The thorny plants spring up around you, entangling your legs!",
	    victim, NULL, NULL, TO_CHAR);

	if (saves_spell(level+2, victim, DAM_PIERCE))
		victim->move = victim->move / 3;
	else
		victim->move = 0;

	if (!is_sn_affected(victim, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= level / 10;
		INT(paf->location) = APPLY_DEX;
		paf->modifier	= -1;
		affect_to_char(victim, paf);
		aff_free(paf);
	}
}

SPELL_FUN(spell_holy_armor, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are already protected from harm.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level;
	INT(paf->location) = APPLY_AC;
	paf->modifier  = - UMAX(10, 10 * (level / 5));
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n is protected from harm.",ch,NULL,NULL,TO_ROOM);
	act_char("You are protected from harm.", ch);

}

SPELL_FUN(spell_love_potion, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 50;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("You feel like looking at people.", ch);
}

SPELL_FUN(spell_protective_shield, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch) {
			act_char("You are already surrounded by a protective shield.", ch);
		} else {
			act("$N is already surrounded by a protective shield.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= number_fuzzy(level / 30) + 3;
	INT(paf->location) = APPLY_AC;
	paf->modifier	= 20;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by a protective shield.",
	    victim, NULL, NULL, TO_ROOM);
	act_char("You are surrounded by a protective shield.", victim);
}

SPELL_FUN(spell_deafen, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (ch == victim) {
		act_char("Deafen who?", ch);
		return;
	}

	if (is_sn_affected(victim,sn)) {
		act("$N is already deaf.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (is_safe_nomessage(ch,victim)) {
		act_char("You cannot deafen that person.", ch);
		return;
	}

	if (saves_spell(level,victim, DAM_NONE))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = 10;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("You have deafened $N!",ch,NULL,victim,TO_CHAR);
	act_char("A loud ringing fills your ears...you can't hear anything!",
		 victim);
}

SPELL_FUN(spell_disperse, sn, level, ch, vo)
{
	CHAR_DATA *vch, *vch_next;
	CHAR_DATA *master = NULL;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You aren't up to dispersing this crowd.", ch);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (vch == ch
		||  !vch->in_room
		||  IS_SET(vch->in_room->room_flags, ROOM_NORECALL)
		||  IS_IMMORTAL(vch))
			continue;

		if (IS_NPC(vch)) {
			if (IS_SET(vch->pMobIndex->act, ACT_AGGRESSIVE)
			||  IS_SET(vch->pMobIndex->act, ACT_IMMSUMMON))
				continue;

                        /*
                        * can't disperse charmed creature if master
                        * is_safe, the same for mounts
                        */

                        if (IS_AFFECTED(vch, AFF_CHARM)
                        &&  vch->master != NULL)
                                master = vch->master;
                        else if (vch->mount != NULL)
                                master = vch->mount;

                        if (master != NULL
                        &&  (is_safe_nomessage(ch, master)))
                                continue;

		} else if (is_safe_nomessage(ch, vch))
				continue;

		teleport_char(vch, NULL, get_random_room(vch, NULL),
			      "$n vanishes!",
			      "The world spins around you!",
			      "$n slowly fades into existence.");
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 15;
	affect_to_char(ch, paf);
	aff_free(paf);
}

SPELL_FUN(spell_honor_shield, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch) {
			act_char("But you're already protected by your honor.",
				 ch);
		} else {
			act("They're already protected by their honor.",
			    ch, NULL, victim, TO_CHAR);
			return;
		}
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	paf->modifier	= -30;
	INT(paf->location) = APPLY_AC;
	affect_to_char(victim, paf);
	aff_free(paf);

	spellfun_call("remove curse", NULL, level, ch, victim);
	spellfun_call("bless", NULL, level, ch, victim);

	act_char("Your honor protects you.", victim);
	act("$n's Honor protects $m.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_dragons_breath, sn, level, ch, vo)
{
	const char *sn_fun;
	act("You call the dragon lord to help you.", ch, NULL, NULL, TO_CHAR);
	act("$n start to breath like a dragon.", ch, NULL, vo, TO_ROOM);

	switch(number_range(1, 5)) {
	case 1:
		sn_fun = "fire breath";
		break;
	case 2:
		sn_fun = "acid breath";
		break;
	case 3:
		sn_fun = "frost breath";
		break;
	case 4:
		sn_fun = "gas breath";
		break;
	case 5:
	default:	/* shut up gcc */
		sn_fun = "lightning breath";
		break;
	}
	spellfun_call(sn_fun, "dragons breath", level, ch, vo);
}

static void *
sand_storm_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch, vch, TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) &&
	     (ch->fighting != vch /*|| vch->fighting != ch*/)))
		return NULL;

	if (saves_spell(level, vch, DAM_COLD)) {
		inflict_effect("sand", vch, level/2, dam/4);
		damage(ch, vch, dam/2, sn, DAM_F_SHOW);
	} else {
		inflict_effect("sand", vch, level, dam);
		damage(ch, vch, dam, sn, DAM_F_SHOW);
	}

	return NULL;
}

SPELL_FUN(spell_sand_storm, sn, level, ch, vo)
{
	int dam, hp_dam, dice_dam;
	int hpch;

	if ((ch->in_room->sector_type == SECT_AIR)
	||  (ch->in_room->sector_type == SECT_WATER_SWIM)
	||  (ch->in_room->sector_type == SECT_WATER_NOSWIM)) {
		act_char("You don't find any sand here to make storm.", ch);
		ch->wait = 0;
		return;
	}

	act("$n creates a storm with sands on the floor.",
	    ch, NULL, NULL, TO_ROOM);
	act("You create the ..sand.. storm.", ch, NULL, NULL, TO_CHAR);

	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch/9 + 1, hpch/5);
	dice_dam = calc_spell_damage(ch, level, sn);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	inflict_effect("sand", ch->in_room, level, dam/2);
	vo_foreach(ch->in_room, &iter_char_room, sand_storm_cb,
		   sn, level, ch, dam);
}

static void *
scream_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;

	if (saves_spell(level, vch, DAM_ENERGY))
		inflict_effect("scream", vch, level/2, dam/4);
	else
		inflict_effect("scream", vch, level, dam);

	if (vch->fighting)
		stop_fighting(vch, TRUE);

	return NULL;
}

SPELL_FUN(spell_scream, sn, level, ch, vo)
{
	int hpch = UMAX(10, ch->hit);
	int hp_dam = number_range(hpch/9 + 1, hpch/5);
	int dice_dam = calc_spell_damage(ch, level, sn);
	int dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam /10);

	act("$n screams with a disturbing NOISE!", ch, NULL, NULL, TO_ROOM);
	act("You scream with a powerful sound.", ch, NULL, NULL, TO_CHAR);

	inflict_effect("scream", ch->in_room, level, dam/2);
	vo_foreach(ch->in_room, &iter_char_room, scream_cb, level, ch, dam);
}

SPELL_FUN(spell_attract_other, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (IS_NPC(ch) || IS_NPC(victim)) {
		act_char("You failed.", ch);
		return;
	}

	if (!mlstr_cmp(&ch->gender, &victim->gender)) {
		act_char("You'd better try your chance on other sex!", ch);
		return;
	}

	spellfun_call("charm person", sn, level+2, ch, vo);
}

#define MOB_VNUM_UNDEAD			18

SPELL_FUN(spell_animate_dead, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *undead;
	int i;
	int chance;
	int u_level;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj, *obj2, *next;
		MOB_INDEX_DATA *undead_idx;
		mlstring ml;
		AFFECT_DATA *paf;

		const char **p;

		obj = (OBJ_DATA *) vo;

		if (!(obj->item_type == ITEM_CORPSE_NPC
		|| obj->item_type == ITEM_CORPSE_PC)) {
			act_char("You can animate only corpses!", ch);
			return;
		}

		if (is_sn_affected(ch, sn)) {
			act_char("You cannot summon the strength to handle more undead bodies.", ch);
			return;
		}

		if (count_charmed(ch))
			return;

		if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
			act_char("You can't animate deads here.", ch);
			return;
		}

		/* can't animate PC corpses in ROOM_BATTLE_ARENA */
		if (obj->item_type == ITEM_CORPSE_PC
		&&  obj->in_room
		&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
		&&  !IS_OWNER(ch, obj)) {
			act_char("You cannot do that.", ch);
			return;
		}

		if (IS_SET(ch->in_room->room_flags,
			   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
			act_char("You can't animate here.", ch);
			return;
		}

		chance = URANGE(5, get_skill(ch, sn)+(level-obj->level)*7, 95);
		if (number_percent() > chance) {
			act_puts("You failed and destroyed it.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n tries to animate $p, but fails and destroys it.",
			    ch, obj, NULL, TO_ROOM);
			for (obj2 = obj->contains; obj2; obj2 = next) {
				next = obj2->next_content;
				obj_to_room(obj2, ch->in_room);
			}
			extract_obj(obj, 0);
			return;
		}

		if ((undead_idx = get_mob_index(MOB_VNUM_UNDEAD)) == NULL)
			return;

		mlstr_init2(&ml, NULL);
		mlstr_cpy(&ml, &obj->owner);

		/*
		 * strip "The undead body of "
		 */
		MLSTR_FOREACH(p, &ml) {
			char buf[MAX_STRING_LENGTH];
			const char *r = mlstr_val(
			    &undead_idx->short_descr, mlstr_lang(&ml, p));
			const char *q;

			if (IS_NULLSTR(*p)
			||  (q = strstr(r, "%s")) == NULL)
				continue;

			strnzncpy(buf, sizeof(buf), r, (size_t) (q-r));
			if (!str_prefix(buf, *p)) {
				const char *s = strdup(*p + strlen(buf));
				free_string(*p);
				*p = s;
			}
		}

		/*
		 * create_mob_of can't return NULL here
		 */
		undead = create_mob_of(MOB_VNUM_UNDEAD, &ml);
		mlstr_destroy(&ml);

		for (i = 0; i < MAX_STAT; i++)
			undead->perm_stat[i] = UMIN(25, 15+obj->level/10);
		u_level = UMIN (obj->level, level+((obj->level-level)/3)*2);

		undead->max_hit = dice(20,u_level*2)+u_level*20;
		undead->hit = undead->max_hit;
		undead->max_mana = dice(u_level,10)+100;
		undead->mana = undead->max_mana;
		undead->alignment = -1000;
		undead->level = u_level;

		for (i = 0; i < 3; i++)
			undead->armor[i] = interpolate(undead->level,100,-100);
		undead->armor[3] = interpolate(undead->level, 50, -200);
		undead->gold = 0;
		undead->silver = 0;
		NPC(undead)->dam.dice_number = 11;
		NPC(undead)->dam.dice_type   = 5;
		undead->damroll  = u_level/2 +10;

		undead->master = ch;
		undead->leader = ch;

		undead->name = str_printf(undead->name, obj->pObjIndex->name);

		for (obj2 = obj->contains; obj2; obj2 = next) {
			next = obj2->next_content;
			obj_to_char(obj2, undead);
		}

		paf = aff_new(TO_AFFECTS, sn);
		paf->level     = level;
		paf->duration  = level / 10;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_puts("With mystic power, you animate it!",
			 ch, NULL, undead, TO_CHAR, POS_DEAD);
		act("With mystic power, $n animates $p!",
		    ch, obj, NULL, TO_ROOM);

		act_puts("$N looks at you and plans to make you "
			 "pay for distrurbing its rest!",
			 ch, NULL, undead, TO_CHAR, POS_DEAD);

		extract_obj(obj, 0);
		char_to_room(undead, ch->in_room);
		if (!IS_EXTRACTED(undead))
			dofun("wear", undead, "all");
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (victim == ch) {
		act_char("But you aren't dead, yet.", ch);
		return;
	}

	act_char("But it ain't dead!!", ch);
}

SPELL_FUN(spell_bone_dragon, sn, level, ch, vo)
{
	CHAR_DATA *coc;
	AFFECT_DATA *paf;
	int i;

	if (!IS_NPC(ch) && PC(ch)->pet) {
		act("You already have a pet.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act("You are still tired from growing previous one.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	if (IS_SET(ch->in_room->room_flags,
		   ROOM_LAW | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("This is not a best place to raise such a monster.",
		    ch);
		return;
	}

	coc = create_mob(MOB_VNUM_COCOON, 0);
	if (coc == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		coc->perm_stat[i] = 5;

	coc->max_hit = number_range (100*level, 200*level);
	coc->hit = coc->max_hit;
	coc->mana = coc->max_mana = 0;
	coc->level = ch->level;
	for (i = 0; i < 4; i++)
		coc->armor[i] = 100 - 2*ch->level - number_range(0, 50);
	coc->gold = 0;
	coc->silver = 0;
	NPC(coc)->dam.dice_number = number_range(1, level/20);
	NPC(coc)->dam.dice_type   = number_range(1, level/10);
	coc->damroll  = number_range(1, level/3);
	coc->master = ch;

	paf = aff_new(TO_AFFECTS, sn);
	paf->duration	= 2 * level / 3;
	affect_to_char(coc, paf);

	paf->duration	= 100;
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(coc, ch->in_room);

	act("Half burrowed cocoon appears from the earth.",
	    ch, NULL, NULL, TO_ALL);
}

SPELL_FUN(spell_enhanced_armor, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already enhancedly armored.", ch);
		else {
			act("$N is already enhancedly armored.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	paf->modifier	= -60;
	INT(paf->location)= APPLY_AC;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel protected from all attacks.", victim);
	if (ch != victim) {
		act("$N is protected by your magic.",
		    ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(spell_meld_into_stone, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch) {
			act_char("Your skin is already covered with stone.",
				 ch);
		} else {
			act("$N's skin is already covered with stone.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level / 10;
	INT(paf->location) = APPLY_AC;
	paf->modifier  = -100;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n's skin melds into stone.", victim, NULL, NULL, TO_ROOM);
	act_char("Your skin melds into stone.", victim);
}

SPELL_FUN(spell_web, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (saves_spell (level, victim, DAM_OTHER))
		return;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already webbed.", ch);
		else
			act("$N is already webbed.", ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 1;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -1 * (level / 6);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DAMROLL;
	paf->modifier	= -1 * (level / 6);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DEX;
	paf->modifier	= -2;
	paf->bitvector	= AFF_WEB;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are emeshed in thick webs!", victim);
	if (ch != victim) {
		act("You emesh $N in a bundle of webs!",
		    ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(spell_mend, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result, skill;

	if (obj->condition > 99) {
		act_char("That item is not in need of mending.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be mended.", ch);
		return;
	}

	skill = get_skill(ch, sn) / 2;
	result = number_percent () + skill;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		result -= 5;
	if (IS_OBJ_STAT(obj, ITEM_MAGIC))
		result += 5;

	if (result >= 50) {
		if (pull_obj_trigger(TRIG_OBJ_REPAIR, obj, ch, NULL) > 0
		||  !mem_is(obj, MT_OBJ)
		||  IS_EXTRACTED(ch))
			return;

		act("$p glows brightly, and is whole again.  Good Job!",
		    ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, and is whole again.",
		    ch, obj, NULL, TO_ROOM);
		obj->condition += result;
		obj->condition = UMIN(obj->condition , 100);
		return;
	} else if (result >= 10) {
		act_char("Nothing seemed to happen.", ch);
		return;
	} else {
		act("$p flares blindingly... and evaporates!",
		    ch, obj, NULL, TO_CHAR);
		act("$p flares blindingly... and evaporates!",
		    ch, obj, NULL, TO_ROOM);
		extract_obj(obj, 0);
		return;
	}
}

SPELL_FUN(spell_shielding, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (saves_spell(level, victim, DAM_NONE)) {
		act("$N shivers slightly, but it passes quickly.",
		    ch, NULL, victim, TO_CHAR);
		act_char("You shiver slightly, but it passes quickly.", victim);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= level / 20;
		affect_join(victim, paf);
		aff_free(paf);

		act("You wrap $N in more flows of Spirit.",
		    ch, NULL, victim, TO_CHAR);
		act_char("You feel the shielding get stronger.", victim);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel as if you have lost touch with something.", victim);
	act("You shield $N from the True Source.", ch, NULL, victim, TO_CHAR);
}

SPELL_FUN(spell_link, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int mana;

	mana = ch->mana / 2;
	ch->mana = 0;
	ch->endur /= 2;
	mana = (mana + number_percent()) / 2;
	victim->mana = victim->mana + mana;
}

SPELL_FUN(spell_power_word_kill, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	bool saves_mental, saves_nega;

	if (ch == victim) {
		act_char("You can't do that to yourself.", ch);
		return;
	}

	act_puts("A stream of darkness from your finger surrounds $N.",
		ch, NULL, victim, TO_CHAR, POS_RESTING);
	act_puts("A stream of darkness from $n's finger surrounds $N.",
		ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("A stream of darkness from $N's finger surrounds you.",
		victim, NULL, ch, TO_CHAR, POS_RESTING);

	saves_mental = saves_spell(level-dice_wlb(1, 10, victim, NULL),
		victim, DAM_MENTAL);
	saves_nega = saves_spell(level-dice_wlb(1, 10, victim, NULL),
		victim, DAM_NEGATIVE);

	if ((saves_mental && saves_nega)
	|| IS_IMMORTAL(victim)
	|| IS_CLAN_GUARD(victim)) {
		act("Your power word doesn't seems to affect $N.",
			ch, NULL, victim, TO_CHAR);
		act("You are not affected by the power word of $n.",
			ch, NULL, victim, TO_VICT);
	} else if (saves_mental || saves_nega) {
		dam = calc_spell_damage(ch, level, sn);
		damage(ch, victim, dam,
		       saves_nega ? "+pwk mental" : sn,  DAM_F_SHOW);
	} else
		raw_kill(ch, victim);
}

#define OBJ_VNUM_EYED_SWORD		88

SPELL_FUN(spell_eyed_sword, sn, level, ch, vo)
{
	OBJ_DATA *eyed;

	eyed = create_obj_of(OBJ_VNUM_EYED_SWORD, &ch->short_descr);
	if (eyed == NULL)
		return;

	eyed->level = ch->level;
	mlstr_cpy(&eyed->owner, &ch->short_descr);
	eyed->ed = ed_new2(eyed->pObjIndex->ed, ch->name);
	INT(eyed->value[2]) = (ch->level / 10) + 3;
	eyed->cost = 0;
	obj_to_char(eyed, ch);
	act_char("You create YOUR sword with your name.", ch);
/*
	act_char("Don't forget that you won't be able to create this weapon anymore.", ch);
*/
}

#define MOB_VNUM_HUNTER			25

SPELL_FUN(spell_lion_help, sn, level, ch, vo)
{
	CHAR_DATA *lion;
	CHAR_DATA *victim;
	AFFECT_DATA *paf;
	char arg[MAX_INPUT_LENGTH];
	int i;

	target_name = one_argument(target_name, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Whom do you want to have killed?", ch);
		return;
	}

	if ((victim = get_char_area(ch,arg)) == NULL) {
		act_char("Noone around with that name.", ch);
		return;
	}

	if (is_safe(ch, victim)) {
		return;
	}

	act_char("You call a hunter lion.", ch);
	act("$n shouts a hunter lion.", ch, NULL, NULL, TO_ROOM);

	if (is_sn_affected(ch, sn)) {
		act_char("You cannot summon the strength to handle more lions right now.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		act_char("No lions can hear you.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL &&
	     ch->in_room->exit[1] == NULL &&
	     ch->in_room->exit[2] == NULL &&
	     ch->in_room->exit[3] == NULL &&
	     ch->in_room->exit[4] == NULL &&
	     ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD &&
	     ch->in_room->sector_type != SECT_FOREST &&
	     ch->in_room->sector_type != SECT_MOUNTAIN &&
	     ch->in_room->sector_type != SECT_HILLS)) {
		act_char("No hunter lion can come to you.", ch);
		return;
	}

	lion = create_mob(MOB_VNUM_HUNTER, 0);
	if (lion == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	lion->max_hit =  UMIN(30000,ch->max_hit * 1.2);
	lion->hit = lion->max_hit;
	lion->max_mana = ch->max_mana;
	lion->mana = lion->max_mana;
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,LEVEL(ch));
	for (i=0; i < 3; i++)
	lion->armor[i] = interpolate(lion->level,100,-100);
	lion->armor[3] = interpolate(lion->level,100,0);
	lion->gold = 0;
	lion->silver = 0;
	NPC(lion)->dam.dice_number = number_range(LEVEL(ch)/15, LEVEL(ch)/10);
	NPC(lion)->dam.dice_type = number_range(LEVEL(ch)/3, LEVEL(ch)/2);
	lion->damroll = number_range(level/8, level/6);

	act_char("A hunter lion comes to kill your victim!", ch);
	act("A hunter lion comes to kill $n's victim!",
	    ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= ch->level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	lion->hunting = victim;
	char_to_room(lion, ch->in_room);
	if (!IS_EXTRACTED(lion))
		return;
	dofun("hunt", lion, str_empty);
}

SPELL_FUN(spell_magic_jar, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *vial;
	OBJ_DATA *fire;

	if (victim == ch) {
		act_char("You like yourself even better.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("Your victim is a npc. Not necessary!", ch);
		return;
	}

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP)) {
		act_char("Seek their soul somewhere else.", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL)) {
		act_char("You failed.", ch);
		return;
	}

	for(vial = ch->carrying; vial != NULL; vial = vial->next_content)
		if (vial->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL)
			break;

	if (vial == NULL)  {
		act_char("You don't have any vials to put your victim's spirit.", ch);
		return;
	}

	fire = create_obj_of(OBJ_VNUM_MAGIC_JAR, &victim->short_descr);
	if (fire == NULL)
		return;

	fire->label = str_qdup(vial->label);
	fire->level = ch->level;
	mlstr_cpy(&fire->owner, &victim->short_descr);
	fire->ed = ed_new2(fire->pObjIndex->ed, victim->name);
	fire->cost = 0;

	extract_obj(vial, 0);
	obj_to_char(fire, ch);
	SET_BIT(PC(victim)->plr_flags, PLR_NOEXP);

	act_puts("You catch $N's spirit into your vial.",
		 ch, NULL, victim, TO_CHAR, POS_DEAD);
	act_puts("$n catches your spirit into vial.",
		 ch, NULL, victim, TO_VICT, POS_DEAD);
}

SPELL_FUN(spell_fear, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!can_flee(victim)) {
		if (victim == ch)
			act_char("You are beyond this power.", ch);
		else
			act_char("Your victim is beyond this power.", ch);
		return;
	}

	if (is_sn_affected(victim, sn)
	||  saves_spell(level, victim, DAM_MENTAL))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	paf->bitvector	= AFF_FEAR;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are afraid as much as a rabbit.", victim);
	act("$n looks with afraid eyes.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_protection_heat, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, "protection heat")) {
		if (victim == ch)
			act_char("You are already protected from heat.", ch);
		else {
			act("$N is already protected from heat.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (is_sn_affected(victim, "protection cold")) {
		if (victim == ch)
			act_char("You are already protected from cold.", ch);
		else {
			act("$N is already protected from cold.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (is_sn_affected(victim, "fire shield")) {
		if (victim == ch)
			act_char("You are already using fire shield.", ch);
		else {
			act("$N is already using fire shield.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_RESISTS, sn);
	paf->level	= level;
	paf->duration	= level * 4 / 3;
	INT(paf->location) = DAM_FIRE;
	paf->modifier	= 25;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel strengthed against heat.", victim);
	if (ch != victim)
		act("$N is protected against heat.", ch, NULL, victim, TO_CHAR);
}

SPELL_FUN(spell_protection_cold, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, "protection cold")) {
		if (victim == ch)
			act_char("You are already protected from cold.", ch);
		else {
			act("$N is already protected from cold.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (is_sn_affected(victim, "protection heat")) {
		if (victim == ch)
			act_char("You are already protected from heat.", ch);
		else {
			act("$N is already protected from heat.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (is_sn_affected(victim, "fire shield")) {
		if (victim == ch)
			act_char("You are already using fire shield.", ch);
		else {
			act("$N is already using fire shield.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_RESISTS, sn);
	paf->level     = level;
	paf->duration  = level * 4 / 3;
	INT(paf->location) = DAM_COLD;
	paf->modifier  = 25;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel strengthed against cold.", victim);
	if (ch != victim)
		act("$N is protected against cold." ,ch, NULL, victim, TO_CHAR);
}

#define OBJ_VNUM_FIRE_SHIELD		92

SPELL_FUN(spell_fire_shield, sn, level, ch, vo)
{
	OBJ_DATA *fire;

	fire = create_obj(OBJ_VNUM_FIRE_SHIELD, 0);
	if (fire == NULL)
		return;

	fire->level = ch->level;
	mlstr_cpy(&fire->owner, &ch->short_descr);

	fire->cost = 0;
	fire->timer = 5 * ch->level ;
	if (IS_GOOD(ch))
		SET_OBJ_STAT(fire, ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL);
	else if (IS_NEUTRAL(ch))
		SET_OBJ_STAT(fire, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
	else if (IS_EVIL(ch))
		SET_OBJ_STAT(fire, ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD);
	obj_to_char(fire, ch);
	act("You create $p.", ch, fire, NULL, TO_CHAR);
}

#define OBJ_VNUM_COLD_SHIELD		79

SPELL_FUN(spell_cold_shield, sn, level, ch, vo)
{
	OBJ_DATA *cold;

	cold = create_obj(OBJ_VNUM_COLD_SHIELD, 0);
	if (cold == NULL)
		return;

	cold->level = ch->level;
	mlstr_cpy(&cold->owner, &ch->short_descr);

	cold->cost = 0;
	cold->timer = 5 * ch->level ;
	if (IS_GOOD(ch))
		SET_OBJ_STAT(cold, ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL);
	else if (IS_NEUTRAL(ch))
		SET_OBJ_STAT(cold, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
	else if (IS_EVIL(ch))
		SET_OBJ_STAT(cold, ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD);
	obj_to_char(cold, ch);
	act("You create $p.", ch, cold, NULL, TO_CHAR);
}

SPELL_FUN(spell_witch_curse, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (is_sn_affected(victim, sn)) {
		act_char("It has already underflowing with health.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)) {
		inflict_spell_damage(ch, victim, level, sn);
		return;
	}

	ch->hit -= (2 * level);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	INT(paf->location)= APPLY_HIT;
	paf->modifier	= - level;
	paf->owner	= ch;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("Now $n got the path to death.", victim, NULL, NULL, TO_ROOM);
	act("Now you got the path to death.", victim, NULL, NULL, TO_CHAR);
}

SPELL_FUN(spell_knock, sn, level, ch, vo)
{
	char arg[MAX_INPUT_LENGTH];
	int chance = 0;
	int door;
	EXIT_DATA *pexit;

	target_name = one_argument(target_name, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Knock which door or direction.", ch);
		return;
	}

	if (ch->fighting) {
		act_char("Wait until the fight finishes.", ch);
		return;
	}

	if ((door = find_door(ch, arg)) < 0)
		return;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		act("It's already open.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		act("Just try to open it.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		act("A mystical shield protects $d.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return;
	}

	chance = level / 5 + get_curr_stat(ch, STAT_INT) + get_skill(ch,sn) / 5;

	act("You knock $d, and try to open it.",
	    ch, &pexit->short_descr, NULL, TO_CHAR);
	act("$n knocks $d, and tries to open it.",
	    ch, &pexit->short_descr, NULL, TO_ROOM);

	if (char_in_dark_room(ch))
		chance /= 2;

	/* now the attack */
	if (number_percent() < chance) {
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit_rev;

		REMOVE_BIT(pexit->exit_info, EX_LOCKED | EX_CLOSED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act("$n knocks $d and opens the lock.",
		    ch, &pexit->short_descr, NULL, TO_ROOM);
		act_puts("You successed to open $d.",
		         ch, &pexit->short_descr, NULL, TO_CHAR, POS_DEAD);

		/* open the other side */
		if ((to_room = pexit->to_room.r) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  pexit_rev->to_room.r == ch->in_room) {
			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED | EX_LOCKED);
			act("$d opens.", to_room->people,
			    &pexit_rev->short_descr, NULL, TO_ROOM);
		}
	} else {
		act("You couldn't knock $d.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		act("$n failed to knock $d.",
		    ch, &pexit->short_descr, NULL, TO_ROOM);
	}
}

SPELL_FUN(spell_hallucination, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (ch == victim) {
			act_char("You are already hallucinating.", ch);
		} else {
			act("$E is already hallucinating.", ch, NULL, victim,
				TO_CHAR);
		}
		return;
	}

	if (saves_spell(level, victim, DAM_MENTAL) && (ch != victim)) {
		act("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 7 + level / 5;
	INT(paf->location)= APPLY_AC;
	paf->modifier	= level * 3;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Wow! Everything looks so different.", victim);

	if (victim != ch)
		act("$N starts hallucinating.", ch, NULL, victim, TO_CHAR);
}

#define MOB_VNUM_WOLF			20

SPELL_FUN(spell_wolf, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA *paf;
	int i;

        if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL &&
	     ch->in_room->exit[1] == NULL &&
	     ch->in_room->exit[2] == NULL &&
	     ch->in_room->exit[3] == NULL &&
	     ch->in_room->exit[4] == NULL &&
	     ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD &&
	     ch->in_room->sector_type != SECT_FOREST &&
	     ch->in_room->sector_type != SECT_MOUNTAIN &&
	     ch->in_room->sector_type != SECT_HILLS)) {
	        act_char("You can't summon a wolf here.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to summon another wolf right now.",
			 ch);
		return;
	}

	act_char("You attempt to summon a wolf.", ch);
	act("$n attempts to summon a wolf.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_WOLF) {
			act_char("Two wolfs are more than you can control!",
				 ch);
			return;
		}
	}

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_LAW | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("But no wolves come.", ch);
		return;
	}

	demon = create_mob(MOB_VNUM_WOLF, 0);
	if (demon == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		demon->perm_stat[i] = ch->perm_stat[i];

	SET_HIT(demon, URANGE(ch->perm_hit, ch->hit, 30000));
	SET_MANA(demon, ch->perm_mana);
	demon->level = ch->level;
	for (i = 0; i < 3; i++)
		demon->armor[i] = interpolate(demon->level,100,-100);
	demon->armor[3] = interpolate(demon->level,100,0);
	demon->gold = 0;
	demon->silver = 0;
	NPC(demon)->dam.dice_number = number_range(level/15, level/10);
	NPC(demon)->dam.dice_type = number_range(level/3, level/2);
	demon->damroll = number_range(level/8, level/6);

	act_char("The wolf arrives and bows before you!", ch);
	act("A wolf arrives from somewhere and bows!", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	demon->master = demon->leader = ch;
	char_to_room(demon, ch->in_room);
}

SPELL_FUN(spell_dragon_skin, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("Your skin is already hard as rock.", ch);
		else {
			act("$N's skin is already hard as rock.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level;
	INT(paf->location) = APPLY_AC;
	paf->modifier	= - (2 * level);
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n's skin is now hard as rock.", victim, NULL, NULL, TO_ROOM);
	act_char("Your skin is now hard as rock.", victim);
}

SPELL_FUN(spell_insanity, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_NPC(victim)) {
		act_char("This spell can cast on PC's only.", ch);
		return;
	}

	if (IS_AFFECTED(victim, AFF_BLOODTHIRST)
	||  saves_spell(level, victim, DAM_MENTAL))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level / 10;
	paf->bitvector = AFF_BLOODTHIRST;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are aggressive as a battlerager.", victim);
	act("$n looks with red eyes.", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_power_stun, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn) || saves_spell(level, victim, DAM_OTHER))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / (LEVEL_HERO - 1);
	INT(paf->location) = APPLY_DEX;
	paf->modifier	= -3;
	paf->bitvector	= AFF_STUN;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are stunned.", victim);
	act_puts("$n is stunned.", victim, NULL, NULL, TO_ROOM, POS_SLEEPING);
}

SPELL_FUN(spell_improved_invis, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_INVIS(victim, ID_IMP_INVIS))
		return;

	if (ch != victim && !has_spec(ch, "major_illusion")) {
		act("You can't cast this spell on another.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_INVIS, sn);
	paf->level	= level;
	paf->duration	= level / 10 ;
	paf->bitvector	= ID_IMP_INVIS;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You fade out of existence.", victim);
}

SPELL_FUN(spell_randomizer, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("Your power of randomness has been exhausted for now.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is far too orderly for your powers to work on it.", ch);
		return;
	}

	if (IS_AFFECTED(ch->in_room, RAFF_RANDOMIZER)) {
		act_char("This room has already been randomized.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;

	if (number_bits(1) == 0) {
		act_char("Despite your efforts, the universe resisted chaos.", ch);
		paf->duration	= level / 10;
		affect_to_char(ch, paf);
	} else {
		paf->type	= sn;
		paf->duration	= level / 5;
		affect_to_char(ch, paf);

		paf->where	= TO_ROOM_AFFECTS;
		paf->duration	= level / 2;
		paf->bitvector	= RAFF_RANDOMIZER;
		paf->owner	= ch;
		affect_to_room(ch->in_room, paf);

		act_char("The room was successfully randomized!", ch);
		act_char("You feel very drained from the effort.", ch);
		ch->hit -= UMIN(200, ch->hit/2);
		act("The room starts to randomize exits.",
		    ch, NULL, NULL, TO_ROOM);
	}

	aff_free(paf);
}

#define MOB_VNUM_FLESH_GOLEM		21

SPELL_FUN(spell_flesh_golem, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA *paf;
	int i = 0;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to create another golem right now.", ch);
		return;
	}

        if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
	        act_char("You can't create a flesh golem here.", ch);
		return;
	}

	act_char("You attempt to create a flesh golem.", ch);
	act("$n attempts to create a flesh golem.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_FLESH_GOLEM) {
			if (++i > 2) {
				act_char("Four flesh golems are more than you can control!", ch);
				return;
			}
		}
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB |
	    ROOM_LAW | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("You create a golem, but it crumples into dust.", ch);
		act("$n creates something, but it crumples into dust", ch,
		    NULL, NULL, TO_ROOM);
		return;
	}

	golem = create_mob(MOB_VNUM_FLESH_GOLEM, 0);
	if (golem == NULL)
		return;

	for (i = 0; i < MAX_STAT; i ++)
		golem->perm_stat[i] = UMIN(25, 15 + level/10);

	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	SET_HIT(golem, UMIN(2 * ch->perm_hit + 400, 30000));
	SET_MANA(golem, ch->perm_mana);
	golem->level = level;
	for (i = 0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	golem->silver = 0;
	NPC(golem)->dam.dice_number = 3;
	NPC(golem)->dam.dice_type = 10;
	golem->damroll = level / 2;

	act_puts("You created $N!", ch, NULL, golem, TO_CHAR, POS_DEAD);
	act("$n creates $N!", ch, NULL, golem, TO_ROOM);


	golem->master = golem->leader = ch;
	char_to_room(golem, ch->in_room);
}

#define MOB_VNUM_STONE_GOLEM		22

SPELL_FUN(spell_stone_golem, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA *paf;
	int i = 0;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to create another golem right now.", ch);
		return;
	}

	act_char("You attempt to create a stone golem.", ch);
	act("$n attempts to create a stone golem.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_STONE_GOLEM) {
			if (++i > 2) {
				act_char("Four stone golems are more than you can control!", ch);
				return;
			}
		}
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB | ROOM_LAW |
	    ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char(
		    "You create a golem, but it crumples into a heap of stone.",
		    ch);
		act(
		   "$n creates something, but it crumples into a heap of stone",
		   ch, NULL, NULL, TO_ROOM);
		return;
	}

	golem = create_mob(MOB_VNUM_STONE_GOLEM, 0);
	if (golem == NULL)
		return;

	for (i = 0; i < MAX_STAT; i ++)
		golem->perm_stat[i] = UMIN(25,15 + level/10);

	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	SET_HIT(golem, UMIN(5 * ch->perm_hit + 2000, 30000));
	SET_MANA(golem, ch->perm_mana);
	golem->level = level;
	for (i = 0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	golem->silver = 0;
	NPC(golem)->dam.dice_number = 8;
	NPC(golem)->dam.dice_type = 4;
	golem->damroll = level / 2;

	act_puts("You created $N!", ch, NULL, golem, TO_CHAR, POS_DEAD);
	act("$n creates $N!", ch, NULL, golem, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	golem->master = golem->leader = ch;
	char_to_room(golem, ch->in_room);
}

#define MOB_VNUM_IRON_GOLEM		23

SPELL_FUN(spell_iron_golem, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA *paf;
	int i = 0;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to create another golem right now.", ch);
		return;
	}

	act_char("You attempt to create an iron golem.", ch);
	act("$n attempts to create an iron golem.", ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_IRON_GOLEM) {
			act_char("Two iron golems are more than you can control!", ch);
			return;
		}
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB | ROOM_LAW |
	    ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char(
		    "You create a golem, but it crumples into a rusty heap.",
		    ch);
		act(
		   "$n creates something, but it crumples into a rusty heap.",
		   ch, NULL, NULL, TO_ROOM);
		return;
	}

	golem = create_mob(MOB_VNUM_IRON_GOLEM, 0);
	if (golem == NULL)
		return;

	for (i = 0; i < MAX_STAT; i ++)
		golem->perm_stat[i] = UMIN(25, 15 + level/10);

	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	SET_HIT(golem, UMIN(10 * ch->perm_hit + 1000, 30000));
	SET_MANA(golem, ch->perm_mana);
	golem->level = level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	golem->silver = 0;
	NPC(golem)->dam.dice_number = 11;
	NPC(golem)->dam.dice_type = 5;
	golem->damroll = level / 2 + 10;

	act_puts("You created $N!", ch, NULL, golem, TO_CHAR, POS_DEAD);
	act("$n creates $N!", ch, NULL, golem, TO_ROOM);

	golem->master = golem->leader = ch;
	char_to_room(golem, ch->in_room);
}

#define MOB_VNUM_ADAMANTITE_GOLEM	24

SPELL_FUN(spell_adamantite_golem, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA *paf;
	int i = 0;

	if (is_sn_affected(ch, sn)) {
		act_char("You lack the power to create another golem right now.", ch);
		return;
	}

        if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
	        act_char("You can't create a adamantite golem here.", ch);
		return;
	}

	act_char("You attempt to create an Adamantite golem.", ch);
	act("$n attempts to create an Adamantite golem.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_ADAMANTITE_GOLEM) {
			act_char("Two adamantite golems are more than you can control!", ch);
			return;
		}
	}

	golem = create_mob(MOB_VNUM_ADAMANTITE_GOLEM, 0);
	if (golem == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		golem->perm_stat[i] = UMIN(25, 15 + level/10);

	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	SET_HIT(golem, UMIN(10 * ch->perm_hit + 4000, 30000));
	SET_MANA(golem, ch->perm_mana);
	golem->level = level;
	for (i = 0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,100,-100);
	golem->armor[3] = interpolate(golem->level,100,0);
	golem->gold = 0;
	golem->silver = 0;
	NPC(golem)->dam.dice_number = 13;
	NPC(golem)->dam.dice_type = 9;
	golem->damroll = level / 2 + 10;

	act_puts("You created $N!", ch, NULL, golem, TO_CHAR, POS_DEAD);
	act("$n creates $N!", ch, NULL, golem, TO_ROOM);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	golem->master = golem->leader = ch;
	char_to_room(golem, ch->in_room);
}

SPELL_FUN(spell_mysterious_dream, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been affected by sleep gas.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be seen good place to sleep.", ch);
	act("The room starts to be seen good place to you.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(spell_ratform, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are rat already.", ch);
		return;
	}

	paf = aff_new(TO_FORM, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	paf->location.s	= str_dup("rat");
	affect_to_char(ch, paf);
	aff_free(paf);

	act("You turn into a small rat for a few hours.",
		ch, NULL, NULL, TO_CHAR);
}

SPELL_FUN(spell_polymorph, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	race_t *r;

	if (is_sn_affected(ch, sn) || is_sn_affected(ch, "deathen")) {
		act_char("You are already polymorphed.", ch);
		return;
	}

	if (target_name == NULL || target_name[0] == '\0') {
		act_char("Usage: cast 'polymorph' <pcracename>.", ch);
		return;
	}

	r = race_search(target_name);
	if (!r || !r->race_pcdata || c_isempty(&r->race_pcdata->classes)) {
		act_char("That is not a valid race to polymorph.", ch);
		return;
	}

	if (IS_SET(r->form, FORM_UNDEAD)) {
		act_char("You posess no necromantic powers to do this.", ch);
		return;
	}

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10;
	paf->location.s	= str_qdup(r->name);
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n polymorphes $mself to $t.", ch, r->name, NULL, TO_ROOM);
	act("You polymorph yourself to $t.", ch, r->name, NULL, TO_CHAR);
}

SPELL_FUN(spell_deathen, sn, level, ch, vo)
{
        AFFECT_DATA *paf;
        CHAR_DATA *victim = (CHAR_DATA *) vo;

        if (is_sn_affected(victim, sn)) {
	        act("$N is already decays.", ch, NULL, victim, TO_CHAR);
	        return;
        }

        if (saves_spell(level,victim, DAM_NEGATIVE)) {
	        act("You failed.", ch, NULL, NULL, TO_CHAR);
	        return;
	}

	paf = aff_new(TO_RACE, sn);
        paf->level	= level;
        paf->duration	= level/15;
        paf->location.s	= str_dup("ghoul");
        affect_to_char(victim, paf);
	aff_free(paf);

        act("$n's flesh starts to decay.", victim, NULL, NULL, TO_ROOM);
        act("Your flesh starts to decay.", victim, NULL, NULL, TO_CHAR);
}

SPELL_FUN(spell_lich, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	race_t *r;
	int lev = 0;

	if (is_sn_affected(ch, sn) || is_sn_affected(ch, "deathen")) {
		act("Your flesh is already dead.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_NULLSTR(target_name)) {
		act_char("Usage: cast lich <type>.", ch);
		return;
	}

	r = race_search(target_name);
	if (!r || !r->race_pcdata || !IS_SET(r->form, FORM_UNDEAD)) {
		act_char("This is not a valid undead type.", ch);
		return;
	}

	if (!str_cmp(r->name, "undead"))
		lev = 0;
	else if (!str_cmp(r->name, "zombie"))
		lev = MAX_LEVEL / 2 ;
	else if (!str_cmp(r->name, "lich"))
		lev = MAX_LEVEL * 2 / 3;

	if (ch->level < lev) {
		act_char("You lack the power to do it.", ch);
		return;
	}

	paf = aff_new(TO_RACE, sn);
	paf->level	= level;
	paf->duration	= level/10;
	paf->location.s	= str_qdup(r->name);
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n deathens $mself, turning into $t.",
	    ch, r->name, NULL, TO_ROOM);
	act("You deathen yourself, turning into $t.",
	    ch, r->name, NULL, TO_CHAR);
}

SPELL_FUN(spell_protection_negative, sn, level, ch, vo)
{
	if (!is_sn_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_RESISTS, sn);
		paf->duration = level / 4;
		paf->level = level;
		INT(paf->location) = DAM_NEGATIVE;
		paf->modifier = 100;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_char("You are now immune to negative attacks.", ch);
	} else
		act_char("You are already immune to negative attacks.", ch);
}

SPELL_FUN(spell_ruler_aura, sn, level, ch, vo)
{
	if (!is_sn_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_RESISTS, sn);
		paf->duration	= level / 4;
		paf->level	= level;
		INT(paf->location) = DAM_CHARM;
		paf->modifier	= 100;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_char("You now feel more self confident in rulership.", ch);
	} else
		act_char("You are as much self confident as you can.", ch);
}

SPELL_FUN(spell_evil_spirit, sn, level, ch, vo)
{
	AREA_DATA *pArea = ch->in_room->area;
	ROOM_INDEX_DATA *room;
	AFFECT_DATA *paf;
	int i;

	if (IS_AFFECTED(ch->in_room, RAFF_ESPIRIT)
	||  is_sn_affected_room(ch->in_room,sn)) {
		act_char("The zone is already full of evil spirit.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("Your power of evil spirit is less for you, now.", ch);		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("Holy aura in this room prevents your powers to work on it.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 5;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 25;
	paf->bitvector	= RAFF_ESPIRIT;
	paf->owner	= ch;

	for (i = pArea->min_vnum; i < pArea->max_vnum; i++) {
		if ((room = get_room_index(i)) == NULL)
			continue;
		affect_to_room(room, paf);

		if (room->people)
			act("The zone is starts to be filled with evil spirit.",
			    room->people, NULL, NULL, TO_ALL);
	}

	aff_free(paf);
}

SPELL_FUN(spell_disgrace, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_sn_affected(victim, sn)
	&&  !saves_spell(level, victim, DAM_MENTAL)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= level;
		INT(paf->location)= APPLY_CHA;
		paf->modifier	= - (5 + level / 10);
		affect_to_char(victim, paf);
		aff_free(paf);

		act("$N feels $M less confident!", ch, NULL, victim, TO_ALL);
	} else
		act_char("You failed.", ch);
}

SPELL_FUN(spell_control_undead, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	race_t *r;

	if (count_charmed(ch))
		return;

	if (victim == ch) {
		act_char("You like yourself even better!", ch);
		return;
	}

	if ((r = race_lookup(victim->race)) == NULL)
		return;

	if  (!IS_SET(victim->form, FORM_UNDEAD)) {
		act("$N doesn't seem to be an undead.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (!IS_NPC(victim) && !IS_NPC(ch))
		level += get_curr_stat(ch, STAT_CHA) -
			 get_curr_stat(victim, STAT_CHA);


	if (IS_IMMORTAL(victim)
	||  IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  saves_spell(level, victim, DAM_OTHER)
	||  (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)
	||  (victim->in_room &&
		IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)))
			return;

	if (is_safe(ch, victim))
		return;

	add_follower(victim, ch);
	set_leader(victim, ch);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= number_fuzzy(level / 5);
	paf->bitvector	= AFF_CHARM;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
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

SPELL_FUN(spell_assist, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("This power is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 50;
	affect_to_char(ch, paf);
	aff_free(paf);

	victim->hit += 100 + level * 5;
	update_pos(victim);
	act_char("A warm feeling fills your body.", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act_char("Ok.", ch);
}

#define MOB_VNUM_SUM_SHADOW		26

SPELL_FUN(spell_summon_shadow, sn, level, ch, vo)
{
	CHAR_DATA *gch;
	CHAR_DATA *shadow;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch,sn)) {
		act_char("You lack the power to summon another shadow right now.", ch);
		return;
	}

	act_char("You attempt to summon a shadow.", ch);
	act("$n attempts to summon a shadow.",ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_SUM_SHADOW) {
			act_char("Two shadows are more than you can control!", ch);
			return;
		}
	}
	if (IS_SET(ch->in_room->room_flags, ROOM_NOMOB | ROOM_LAW |
	    ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("The shadows aren't deep enough here.", ch);
		return;
	}

	shadow = create_mob(MOB_VNUM_SUM_SHADOW, 0);
	if (shadow == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		shadow->perm_stat[i] = ch->perm_stat[i];

	SET_HIT(shadow, URANGE(ch->perm_hit, ch->hit, 30000));
	SET_MANA(shadow, ch->perm_mana);
	shadow->level = level;
	for (i = 0; i < 3; i++)
		shadow->armor[i] = interpolate(shadow->level, 100, -100);
	shadow->armor[3] = interpolate(shadow->level,100,0);
	shadow->gold = 0;
	shadow->silver = 0;
	NPC(shadow)->dam.dice_number = number_range(level/15, level/10);
	NPC(shadow)->dam.dice_type = number_range(level/3, level/2);
	shadow->damroll = number_range(level/8, level/6);

	act("A shadow conjures!", ch, NULL, NULL, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	shadow->master = shadow->leader = ch;
	char_to_room(shadow,ch->in_room);
}

SPELL_FUN(spell_farsight, sn, level, ch, vo)
{
	ROOM_INDEX_DATA *room;

	if (IS_NULLSTR(target_name)) {
		act("Farsight which direction?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((room = check_place(ch, target_name)) == NULL) {
		act_char("You cannot see that much far.", ch);
		return;
	}

	if (ch->in_room == room)
		dofun("look", ch, "auto");
	else
		look_at(ch, room);
}

#define MOB_VNUM_MIRROR_IMAGE		17

SPELL_FUN(spell_mirror, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf, *paf2;
	int mirrors, new_mirrors;
	CHAR_DATA *gch;
	int order;

	if (IS_NPC(victim)) {
		act_char("Only players can be mirrored.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_char("You failed.", ch);
		return;
	}

	for (mirrors = 0, gch = npc_list; gch; gch = gch->next)
		if (is_sn_affected(gch, "mirror")
		&&  is_sn_affected(gch, "doppelganger")
		&&  gch->doppel == victim)
			mirrors++;

	if (mirrors >= level/5) {
		if (ch == victim)
			act_char("You cannot be further mirrored.", ch);
		else
			act("$N cannot be further mirrored.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= -1;

	paf2 = aff_new(TO_AFFECTS, "doppelganger");
	paf->level	= level;
	paf->duration	= level;

	order = number_range(0, level/5 - mirrors);

	for (new_mirrors = 0; mirrors + new_mirrors < level/5; new_mirrors++) {
		gch = create_mob(MOB_VNUM_MIRROR_IMAGE, 0);
		if (gch == NULL)
			break;

		free_string(gch->name);
		gch->name = str_qdup(victim->name);
		mlstr_cpy(&gch->short_descr, &victim->short_descr);
		mlstr_printf(&gch->long_descr, &gch->pMobIndex->long_descr,
			     victim->name, PC(victim)->title);
		mlstr_cpy(&gch->description, &victim->description);
		mlstr_cpy(&gch->gender, &victim->gender);

		affect_to_char(gch, paf);
		affect_to_char(gch, paf2);

		gch->max_hit = gch->hit = 1;
		gch->level = 1;
		gch->doppel = victim;
		gch->master = victim;

		if (ch == victim) {
			act_char("A mirror image of yourself appears beside you!", ch);
			act("A mirror image of $n appears beside $M!",
			    ch, NULL, victim, TO_ROOM);
		} else {
			act("A mirror of $N appears beside $M!",
			    ch, NULL, victim, TO_CHAR);
			act("A mirror of $N appears beside $M!",
			    ch,NULL,victim,TO_NOTVICT);
			act_char("A mirror image of yourself appears beside you!", victim);
		}

		char_to_room(gch, victim->in_room);
		if (new_mirrors == order) {
			char_to_room(victim, gch->in_room);
			if (IS_EXTRACTED(victim))
				break;
		}
	}

	aff_free(paf);
	aff_free(paf2);
}

SPELL_FUN(spell_doppelganger, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (ch == victim || (is_sn_affected(ch, sn) && ch->doppel == victim)) {
		act("You already look like $M.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_NPC(victim)) {
		act("$N is too different from you to mimic.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_char("Yeah, sure. And I'm the Pope.", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_CHARM)) {
		act_char("You failed.", ch);
		return;
	}

	act("You change form to look like $N.", ch, NULL, victim, TO_CHAR);
	act("$n changes form to look like YOU!", ch, NULL, victim, TO_VICT);
	act("$n changes form to look like $N!", ch, NULL, victim, TO_NOTVICT);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 2 * level / 3;
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->doppel = victim;
}

SPELL_FUN(spell_hunger_weapon, sn, level, ch, vo)
{
        OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;

        if (obj->item_type != ITEM_WEAPON) {
		act_char("That's not a weapon.", ch);
		return;
        }

        if (obj->wear_loc != WEAR_NONE) {
		act_char("The item must be carried to be cursed.", ch);
		return;
        }

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) {
		act("The gods are infuriated!", ch, NULL, NULL, TO_ALL);
		damage(ch, ch, (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1),
		       "+divine", DAM_F_SHOW);
		return;
	}

        if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) {
		act("$p is already hungry for enemy life.",
		    ch, obj, NULL, TO_CHAR);
		return;
        }

	chance = get_skill(ch, sn);

	if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))	chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_FROST))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHARP))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_VORPAL))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHOCKING))	chance /= 2;

        if (number_percent() < chance) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_WEAPON, sn);
		paf->level	= level / 2;
		paf->duration	= level/8;
		paf->bitvector	= WEAPON_VAMPIRIC;
		affect_to_obj(obj, paf);
		aff_free(paf);

		SET_OBJ_STAT(obj, ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
		act("You transmit part of your hunger to $p.",
		    ch, obj, NULL, TO_CHAR);
	} else
		act("You failed.", ch, obj, NULL, TO_CHAR);
}

/*
 * An alteration spell which enlarges a given person
 */
SPELL_FUN(spell_enlarge, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (is_sn_affected(victim, sn)) {
		if (ch==victim)
			act("You are already as large as you can get.",
				ch, NULL, NULL, TO_CHAR);
		else
			act("$N is already as large as $E can get.",
				ch, NULL, victim, TO_CHAR);
		return;
	}

	if (check_trust(ch, victim)
	||  !saves_spell(level, victim, DAM_NEGATIVE)) {
		AFFECT_DATA *paf;

		if (is_sn_affected(victim, "shrink")) {
			affect_strip(victim, "shrink");
			act("You grow back to your normal size.",
				victim, NULL, NULL, TO_CHAR);
			act("$n grows back to $s normal size.",
				victim, NULL, NULL, TO_ROOM);
			return;
		}

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= 5 + level/10;

		INT(paf->location)= APPLY_SIZE;
		paf->modifier	= 1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_STR;
		paf->modifier	= 1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DEX;
		paf->modifier	= -1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DAMROLL;
		paf->modifier	= level/18 + 1;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("Your body suddenly becomes much larger.",
			victim, NULL, NULL, TO_CHAR);
		act("$n suddenly seems to become much larger.",
			victim, NULL, NULL, TO_ROOM);
	} else
		act_char("You failed.", ch);
}

/*
 * A reversed enlarge spell.
 */
SPELL_FUN(spell_shrink, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (is_sn_affected(victim, sn)) {
		if (ch == victim) {
			act("You are already as small as you can get.",
				ch, NULL, NULL, TO_CHAR);
		} else {
			act("$N is already as small as $E can get.",
				ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (check_trust(ch, victim)
	||  !saves_spell(level, victim, DAM_NEGATIVE)) {
		AFFECT_DATA *paf;

		if (is_sn_affected(victim, "enlarge")) {
			affect_strip(victim, "enlarge");
			act("You shrink back to your normal size.",
				victim, NULL, NULL, TO_CHAR);
			act("$n shrink back to $s normal size.",
				victim, NULL, NULL, TO_ROOM);
			return;
		}

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= 5 + level/10;

		INT(paf->location)= APPLY_SIZE;
		paf->modifier	= -1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_STR;
		paf->modifier	= -1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DEX;
		paf->modifier	= 1;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= level/18 + 1;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("Your body suddenly becomes much smaller.",
			victim, NULL, NULL, TO_CHAR);
		act("$n suddenly seems to become much smaller.",
			victim, NULL, NULL, TO_ROOM);
	} else
		act_char("You failed.", ch);
}

SPELL_FUN(spell_water_breathing, sn, level, ch, vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(vch, sn)) {
		if (ch == vch)
			act_char("You can already breath under water.", ch);
		else {
			act("$E can already breath under water.",
				ch, NULL, vch, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 12;
	affect_to_char(vch, paf);
	aff_free(paf);

	act_char("Breathing seems a bit easier.", vch);
}

SPELL_FUN(spell_blur, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("Your body is already blurred.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 12 + 2;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n's body becomes blurred.", ch, NULL, NULL, TO_ROOM);
	act("Your body becomes blurred.", ch, NULL, NULL, TO_CHAR);
}

#define MOB_VNUM_BLACK_CAT		30
#define MOB_VNUM_BLACK_CROW		31

SPELL_FUN(spell_find_familiar, sn, level, ch, vo)
{
	CHAR_DATA *familiar=NULL;
	CHAR_DATA *gch;
	int i;

	int chance;
	int vnum;
	bool new=TRUE;
	chance = number_percent();

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL &&
	     ch->in_room->exit[1] == NULL &&
	     ch->in_room->exit[2] == NULL &&
	     ch->in_room->exit[3] == NULL &&
	     ch->in_room->exit[4] == NULL &&
	     ch->in_room->exit[5] == NULL)) {
	        act_char("You can't find a familiar.", ch);
		return;
	}

	if (chance < 60)
		vnum = MOB_VNUM_BLACK_CAT;
	else
		vnum = MOB_VNUM_BLACK_CROW;

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_SET(gch->pMobIndex->act, ACT_FAMILIAR)
		&&  gch->master == ch) {
			familiar = gch;
			new = FALSE;
			break;
		}
	}

	if (!familiar) {
		familiar = create_mob(vnum, 0);
		if (familiar == NULL)
			return;

		switch(vnum) {
		case MOB_VNUM_BLACK_CAT:
			familiar->perm_stat[STAT_STR] = 12;
			familiar->perm_stat[STAT_INT] = 23;
			familiar->perm_stat[STAT_DEX] = 24;
			familiar->perm_stat[STAT_WIS] = 12;
			familiar->perm_stat[STAT_CON] = 14;
			familiar->perm_stat[STAT_CHA] =
			    get_curr_stat(ch, STAT_CHA)-1;
			break;
		case MOB_VNUM_BLACK_CROW:
			familiar->perm_stat[STAT_STR] = 13;
			familiar->perm_stat[STAT_INT] = 18;
			familiar->perm_stat[STAT_DEX] = 14;
			familiar->perm_stat[STAT_WIS] = 24;
			familiar->perm_stat[STAT_CON] = 13;
			familiar->perm_stat[STAT_CHA] =
			    get_curr_stat(ch, STAT_CHA)-4;
			break;
		}

		/* Randomize stats a bit */
		for (i = 0; i < MAX_STAT; i++)
			familiar->perm_stat[i] += number_range(-1, 1);
	}

	familiar->max_hit =
	    ch->max_hit/2 + (ch->max_hit)*get_curr_stat(familiar, STAT_CON)/50;
	familiar->max_mana =
	    ch->max_mana * (get_curr_stat(familiar, STAT_INT) +
			    get_curr_stat(familiar, STAT_WIS))/48;
	familiar->level = ch->level;
	familiar->hit = familiar->max_hit;
	familiar->mana = familiar->max_mana;
	for (i=0; i<4; i++)
		familiar->armor[i] = interpolate(familiar->level,100,-100);

	familiar->gold = 0;
	familiar->silver = 0;

	NPC(familiar)->dam.dice_number = 5;
	NPC(familiar)->dam.dice_type   = 5;

	familiar->damroll = level/3;

	familiar->master = familiar->leader = ch;

	if (!new) {
		teleport_char(familiar, ch, ch->in_room,
			"$n disappears suddenly.",
			"$N has summoned you!",
			"$n arrives suddenly.");
	} else {
		char_to_room(familiar, ch->in_room);

		act("You attempt to find a familiar.", ch, NULL, NULL, TO_CHAR);
		act("$N comes to serve you.", ch, NULL, familiar, TO_CHAR);
		act("$N comes to serve $n.", ch, NULL, familiar, TO_NOTVICT);
	}
}

/* function for some necromancers area-attack */
static void *
death_ripple_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);
	int door = va_arg(ap, int);
	int *pcounter = va_arg(ap, int *);

	if (is_safe_spell(ch, vch, TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch))
	||  (IS_NPC(vch) && IS_SET(vch->pMobIndex->act, ACT_NOTRACK)))
		return NULL;

	(*pcounter)++;
	if (saves_spell(level, vch, DAM_NEGATIVE))
		dam /= 2;
	damage(ch, vch, dam, sn, DAM_F_SHOW);

	if (door != -1 && IS_NPC(vch))
		path_to_track(ch, vch, door);
	return NULL;
}

SPELL_FUN(spell_death_ripple, sn, level, ch, vo)
{
	ROOM_INDEX_DATA *this_room;
	int v_counter, range, door, i;
	int dam;
	this_room = ch->in_room;
	range = level/10;

	act("You feel $n's deadly wave passing through your body.",
	    ch, NULL, NULL, TO_ROOM);
        act("Deadly wave emanates from you.",
	    ch, NULL, NULL, TO_CHAR);
	dam = calc_spell_damage(ch, level, sn);
	vo_foreach(this_room, &iter_char_room, death_ripple_cb,
		   sn, level, ch, dam, -1, &v_counter);

        if (!check_blind(ch))
		return;

	if ((door = exit_lookup(target_name)) < 0) {
                act_char("Which direction?", ch);
                return;
        }

        for (i = 1; i <= range; i++) {
		ROOM_INDEX_DATA *next_room;
		EXIT_DATA *to_next, *to_this;

                if ((to_next = this_room->exit[door]) == NULL
                ||  IS_SET(to_next->exit_info, EX_CLOSED)
                ||  (next_room = to_next->to_room.r) == NULL
                ||  !can_see_room(ch, next_room)
		||  (to_this = next_room->exit[rev_dir[door]]) == NULL
		||  IS_SET(to_this->exit_info, EX_CLOSED)
		||  to_this->to_room.r != this_room
		||  v_counter > level/10)
			return;

		act("You feel someone's breath from $T.",
		    next_room->people, NULL, from_dir_name[rev_dir[door]],
		    TO_ALL);
		this_room = next_room;
		vo_foreach(this_room, &iter_char_room, death_ripple_cb,
			   sn, level, ch, dam, door, &v_counter);
	}
}

SPELL_FUN(spell_simulacrum, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *illusion;
	CHAR_DATA *gch;
	int hitp, i;
	AFFECT_DATA *paf;
	int count_illusions = 0;

	if (!IS_NPC(victim) || (LEVEL(victim) - level) > 10) {
		act_char("You failed.", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (gch->master == ch
		&& (is_sn_affected(gch, sn)))
			count_illusions++;
	}

	if (count_illusions > 0 ) {
		act("You already control a simulacrum.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	/*
	 * create_mob can't return NULL here
	 */
	illusion = create_mob(victim->pMobIndex->vnum, 0);
	hitp = victim->hit;
	hitp /= 2;

	hitp += hitp * number_range(1, 10) / 10;
	illusion->max_hit = illusion->hit = hitp;
	illusion->level = victim->level;

	for (i = 0; i < MAX_RESIST; i++) {
		illusion->res_mod[i] = 0;
	}
	illusion->res_mod[DAM_POISON] = illusion->res_mod[DAM_DISEASE] = 100;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->bitvector	= AFF_CHARM;
	affect_to_char(illusion, paf);
	aff_free(paf);

	act("You create a simulacrum of $N.", ch, NULL, victim, TO_CHAR);
	act("$n creates a simulacrum of $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n creates an exact copy of you!", ch, NULL, victim, TO_VICT);

	illusion->master = illusion->leader = ch;
	char_to_room(illusion, ch->in_room);
}

SPELL_FUN(spell_misleading, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act("You are already affected.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= level;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("You will now mislead the people who follow you.",
		ch, NULL, NULL, TO_CHAR);
}

SPELL_FUN(spell_phantasmal_force, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int saves_count = 0;
	int dam = 0;
	int dam_total = 0;
	int i;
	int count;

	if (ch == victim) {
		act("You wouldn't believe your own illusions.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	for (i = 0; i < 3; i++)
		if (saves_spell(level, victim, DAM_MENTAL))
			saves_count++;

	if (saves_count > 3) {
		act("$E didn't believe.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (level < MAX_LEVEL / 3 * 2) {
		switch(saves_count) {
		case 3:
			act("A bee swarm arrives from nowhere and attacks you!",
				ch, NULL, victim, TO_VICT);
			count = number_range(8, 15);
			for (i = 0; i < count; i++) {
				dam = dice(2, 2);
				dam_total += dam;
				act_puts3("$W $u you!",
					  ch, vp_dam_alias(dam), victim,
					  "bee bite",		// notrans
					  TO_VICT, POS_DEAD);
			}
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam_total), victim,
				  "bee swarm",			// notrans
				  TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;
		case 2:
			act("Group of goblins jumps out their ambush towards you!", ch, NULL, victim, TO_VICT);
			count = number_range(2, 5);
			for (i = 0; i < count; i++) {
				dam = dice(level, 4);
				dam_total += dam;
				act_puts3("$W $u you!",
					  ch, vp_dam_alias(dam), victim,
					  "goblin's punch",	// notrans
					  TO_VICT, POS_DEAD);
			}
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam_total), victim,
				  "troop of goblins",		// notrans
				  TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;
		case 1:
			act("Large troll has arrived.",
			    ch, NULL, victim, TO_VICT);
			count = number_range(2, 4);
			for (i = 0; i < count; i++) {
				dam = dice(level, 6);
				dam_total += dam;
				act_puts3("$W $u you!",
					  ch, vp_dam_alias(dam), victim,
					  "troll's smash",	// notrans
					  TO_VICT, POS_DEAD);
			}
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam_total), victim,
				  "troll", TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;
		case 0:
		default:
			act("$n turns into dragon and breathes fire at you!",
			    ch, NULL, victim, TO_VICT);
			dam = dice(level, 15);
			act_puts3("$n's $W $u you!",
				  ch, vp_dam_alias(dam), victim,
				  "fire breath",		// notrans
				  TO_VICT, POS_DEAD);
			act("$N believes you turn yourself into dragon and breathe fire at $M.", ch, NULL, victim, TO_CHAR);
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam), victim,
				  "fire breath", TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			inflict_effect("fire", victim, level, 0);
			return;
		}
	} else {
		switch(saves_count) {
		case 3:
			act("$n conjures a large ball of fire!",
				ch, NULL, victim, TO_VICT);
			dam = dice(level, 12);
			act_puts3("$n's $W $u you!",
				  ch, vp_dam_alias(dam), victim,
				  "fireball", TO_VICT, POS_DEAD);
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam), victim,
				  "fireball", TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;
		case 2:
			act("The earth trembles beneath your feet.",
				ch, NULL, victim, TO_VICT);
			dam = dice(level, 15);
			act_puts3("$W $u you!",
				  ch, vp_dam_alias(dam), victim,
				  "earthquake", TO_VICT, POS_DEAD);
			act_puts3("Your illusionary $W $u $N!",
				  ch, vp_dam_alias(dam), victim,
				  "earthquake", TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;

		case 1:
			act("A demon prince arrives from the puff of smoke.",
				ch, NULL, victim, TO_VICT);
			count = number_range(2, 4);
			for (i = 0; i < count; i++) {
				dam = dice(level, 9);
				dam_total += dam;
				act_puts3("$W $u you!",
					  ch, vp_dam_alias(dam), victim,
					  "demon prince",	// notrans
					  TO_VICT, POS_DEAD);
			}
			act_puts3("Illusionary $W $u $N!",
				  ch, vp_dam_alias(dam_total), victim,
				  "demon prince",		// notrans
				  TO_CHAR, POS_DEAD);
			damage(ch, victim, dam_total, sn, DAM_F_NOREDUCE);
			return;

		case 0:
			act("A large rock falls on you from the sky.",
			    ch, NULL, victim, TO_VICT);
			act("You are squeezed by the rock.",
			    ch, NULL, victim, TO_VICT);
			act("$N believes $E is squeezed by the large rock fallen from the sky and dies.", ch, NULL, victim, TO_CHAR);
			act("$n makes $N believe that large rock from the sky squeezes $M.", ch, NULL, victim, TO_NOTVICT);
			raw_kill(ch, victim);
		default:
			return;
		}

	}
}

/*
 * Spell: 'protection from missiles'.
 * School: abjuration
 * Target: self
 * Effect: makes caster totally immune to all the normal missiles
 * Level: medium
 */
SPELL_FUN(spell_protection_from_missiles, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are already well protected from missiles.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 4;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("You feel well protected from range attacks.", ch);
}

/*
 * Spell: 'alarm'
 * School: abjuration
 * Target: room
 * Effect: creates an alarm, which will inform everyone in the room about
 * creature arrived.
 * Level: medium
 */
SPELL_FUN(spell_alarm, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("The alarm in this room had been already set.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("You set a small alarm in the room.", ch);
}

/* Spell: 'globe of invulnerability'
 * School: abjuration
 * Target: self
 * Effect: protects caster from any spell, but shatters if character moves
 * or recieves damage.
 * Level: medium-high
 */
SPELL_FUN(spell_globe_of_invulnerability, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are already protected by a globe.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 2 + level / 14;
	INT(paf->location)= APPLY_SAVES;
	paf->modifier	= -2 - level / 10;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("$n is surrounded by the globe of invulnerability.", ch, NULL, NULL,
	    TO_ROOM);
	act_char("You are surrounded by the globe of invulnerability.", ch);
}

/* Spell 'cloak of leaves'
 * Quest item spell
 * Target: self
 * Effect: increases health of caster
 */

SPELL_FUN(spell_cloak_of_leaves, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	int hp_to_add;

	if (is_sn_affected(ch,sn)) {
		act_char("You are already protected.", ch);
		return;
	}
	hp_to_add = number_range(level * 3 / 4, level * 4 / 3) + 15;
	ch->hit += hp_to_add;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level		= level;
	paf->duration		= level / 3 + 3;
	INT(paf->location)	= APPLY_HIT;
	paf->modifier		= hp_to_add;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Many green leaves cloak you.",ch);
}
