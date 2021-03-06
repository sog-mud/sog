/*
 * $Id: prayers.c,v 1.86 2004-10-27 12:38:10 sg Exp $
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

#include <merc.h>

#include <sog.h>
#include <quest.h>

DECLARE_SPELL_FUN(prayer_detect_good);
DECLARE_SPELL_FUN(prayer_detect_evil);
DECLARE_SPELL_FUN(prayer_detect_poison);
DECLARE_SPELL_FUN(prayer_refresh);
DECLARE_SPELL_FUN(prayer_restoring_light);
DECLARE_SPELL_FUN(prayer_word_of_recall);
DECLARE_SPELL_FUN(prayer_inspire);
DECLARE_SPELL_FUN(prayer_heal);
DECLARE_SPELL_FUN(prayer_master_healing);
DECLARE_SPELL_FUN(prayer_group_heal);
DECLARE_SPELL_FUN(prayer_superior_heal);
DECLARE_SPELL_FUN(prayer_mass_healing);
DECLARE_SPELL_FUN(prayer_dispel_good);
DECLARE_SPELL_FUN(prayer_dispel_evil);
DECLARE_SPELL_FUN(prayer_cure_light_wounds);
DECLARE_SPELL_FUN(prayer_cure_serious_wounds);
DECLARE_SPELL_FUN(prayer_cure_critical_wounds);
DECLARE_SPELL_FUN(prayer_cure_blindness);
DECLARE_SPELL_FUN(prayer_cure_poison);
DECLARE_SPELL_FUN(prayer_cure_disease);
DECLARE_SPELL_FUN(prayer_group_defence);
DECLARE_SPELL_FUN(prayer_turn);
DECLARE_SPELL_FUN(prayer_mana_restore);
DECLARE_SPELL_FUN(prayer_severity_force);
DECLARE_SPELL_FUN(prayer_create_spring);
DECLARE_SPELL_FUN(prayer_create_food);
DECLARE_SPELL_FUN(prayer_create_water);
DECLARE_SPELL_FUN(prayer_resilience);
DECLARE_SPELL_FUN(prayer_mass_sanctuary);
DECLARE_SPELL_FUN(prayer_lightning_bolt);
DECLARE_SPELL_FUN(prayer_sanctify_lands);
DECLARE_SPELL_FUN(prayer_cursed_lands);
DECLARE_SPELL_FUN(prayer_desert_fist);
DECLARE_SPELL_FUN(prayer_calm);
DECLARE_SPELL_FUN(prayer_harm);
DECLARE_SPELL_FUN(prayer_wrath);
DECLARE_SPELL_FUN(prayer_bless);
DECLARE_SPELL_FUN(prayer_inflict_light_wounds);
DECLARE_SPELL_FUN(prayer_inflict_serious_wounds);
DECLARE_SPELL_FUN(prayer_inflict_critical_wounds);
DECLARE_SPELL_FUN(prayer_demonfire);
DECLARE_SPELL_FUN(prayer_mind_wrack);
DECLARE_SPELL_FUN(prayer_remove_fear);
DECLARE_SPELL_FUN(prayer_call_lightning);
DECLARE_SPELL_FUN(prayer_protection_good);
DECLARE_SPELL_FUN(prayer_protection_evil);
DECLARE_SPELL_FUN(prayer_restoration);
DECLARE_SPELL_FUN(prayer_holy_hammer);
DECLARE_SPELL_FUN(prayer_hold_person);
DECLARE_SPELL_FUN(prayer_lethargic_mist);
DECLARE_SPELL_FUN(prayer_mind_wrench);
DECLARE_SPELL_FUN(prayer_holy_word);
DECLARE_SPELL_FUN(prayer_healing_light);
DECLARE_SPELL_FUN(prayer_sanctuary);
DECLARE_SPELL_FUN(prayer_black_shroud);
DECLARE_SPELL_FUN(prayer_solar_flight);
DECLARE_SPELL_FUN(prayer_black_death);
DECLARE_SPELL_FUN(prayer_etheral_fist);
DECLARE_SPELL_FUN(prayer_meteor_shower);
DECLARE_SPELL_FUN(prayer_blade_barrier);
DECLARE_SPELL_FUN(prayer_anathema);
DECLARE_SPELL_FUN(prayer_heat_metal);
DECLARE_SPELL_FUN(prayer_mind_light);
DECLARE_SPELL_FUN(prayer_free_action);
DECLARE_SPELL_FUN(prayer_deadly_venom);
DECLARE_SPELL_FUN(prayer_ray_of_truth);
DECLARE_SPELL_FUN(prayer_aid);
DECLARE_SPELL_FUN(prayer_bluefire);
DECLARE_SPELL_FUN(prayer_bless_weapon);
DECLARE_SPELL_FUN(prayer_control_weather);
DECLARE_SPELL_FUN(prayer_benediction);
DECLARE_SPELL_FUN(prayer_curse);
DECLARE_SPELL_FUN(prayer_remove_curse);
DECLARE_SPELL_FUN(prayer_flamestrike);
DECLARE_SPELL_FUN(prayer_know_alignment);
DECLARE_SPELL_FUN(prayer_frenzy);
DECLARE_SPELL_FUN(prayer_awaken);
DECLARE_SPELL_FUN(prayer_shapechange);
DECLARE_SPELL_FUN(prayer_hold_animal);
DECLARE_SPELL_FUN(prayer_light);
DECLARE_SPELL_FUN(prayer_darkness);
DECLARE_SPELL_FUN(prayer_air_elemental);
DECLARE_SPELL_FUN(prayer_wall_of_thorns);
DECLARE_SPELL_FUN(prayer_obscuring_mist);
DECLARE_SPELL_FUN(prayer_treeform);
DECLARE_SPELL_FUN(prayer_fly);
DECLARE_SPELL_FUN(prayer_mist_walk);
DECLARE_SPELL_FUN(prayer_air_walk);
DECLARE_SPELL_FUN(prayer_nightmare);
DECLARE_SPELL_FUN(prayer_abolish_undead);
DECLARE_SPELL_FUN(prayer_golden_aura);
DECLARE_SPELL_FUN(prayer_fire_sphere);
DECLARE_SPELL_FUN(prayer_sunburst);
DECLARE_SPELL_FUN(prayer_cone_of_cold);
DECLARE_SPELL_FUN(prayer_power_word_fear);
DECLARE_SPELL_FUN(prayer_windwall);
DECLARE_SPELL_FUN(prayer_wail_of_the_banshee);
DECLARE_SPELL_FUN(prayer_sunbeam);
DECLARE_SPELL_FUN(prayer_acid_fog);
DECLARE_SPELL_FUN(prayer_ice_storm);
DECLARE_SPELL_FUN(prayer_blast_of_wind);
DECLARE_SPELL_FUN(prayer_fog_cloud);
DECLARE_SPELL_FUN(prayer_water_ward);
DECLARE_SPELL_FUN(prayer_air_ward);
DECLARE_SPELL_FUN(prayer_earth_ward);
DECLARE_SPELL_FUN(prayer_fire_ward);
DECLARE_SPELL_FUN(prayer_death_ward);
DECLARE_SPELL_FUN(prayer_spear_of_death);
DECLARE_SPELL_FUN(prayer_sleep_of_grave);
DECLARE_SPELL_FUN(prayer_wind_blow);
DECLARE_SPELL_FUN(prayer_tornado);
DECLARE_SPELL_FUN(prayer_ice_sphere);
DECLARE_SPELL_FUN(prayer_cloak_of_death);
DECLARE_SPELL_FUN(prayer_water_walk);
DECLARE_SPELL_FUN(prayer_breath_under_water);
DECLARE_SPELL_FUN(prayer_geyser);
DECLARE_SPELL_FUN(prayer_water_elemental);
DECLARE_SPELL_FUN(prayer_fire_elemental);
DECLARE_SPELL_FUN(prayer_earth_elemental);
DECLARE_SPELL_FUN(prayer_produce_flame);
DECLARE_SPELL_FUN(prayer_death_breathing);
DECLARE_SPELL_FUN(prayer_rock_skin);
DECLARE_SPELL_FUN(prayer_spike_stones);
DECLARE_SPELL_FUN(prayer_strength_of_earth);
DECLARE_SPELL_FUN(prayer_soil_barbs);
DECLARE_SPELL_FUN(prayer_earthquake);
DECLARE_SPELL_FUN(prayer_hail_of_stones);
DECLARE_SPELL_FUN(prayer_incendiary_cloud);
DECLARE_SPELL_FUN(prayer_unholy_aura);
DECLARE_SPELL_FUN(prayer_unholy_blight);
DECLARE_SPELL_FUN(prayer_inspire_weapon);
DECLARE_SPELL_FUN(prayer_blasphemy);

static void
hold(CHAR_DATA *ch, CHAR_DATA *victim, int duration, int dex_modifier, int
    level);

static bool
can_cast_sanctuary(CHAR_DATA *ch, CHAR_DATA *victim);

static bool
can_cast_wards(CHAR_DATA *ch, CHAR_DATA *victim);

SPELL_FUN(prayer_detect_good, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_GOOD)) {
		if (victim == ch)
			act_char("You can already sense good.", ch);
		else {
			act("$N can already detect good.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_GOOD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(prayer_detect_evil, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (HAS_DETECT(victim, ID_EVIL)) {
		if (victim == ch)
			act_char("You can already sense evil.", ch);
		else {
			act("$N can already detect evil.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_DETECTS, sn);
	paf->level	= level;
	paf->duration	= (5 + level / 3);
	paf->bitvector	= ID_EVIL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

SPELL_FUN(prayer_detect_poison, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if (obj->pObjIndex->item_type == ITEM_DRINK_CON || obj->pObjIndex->item_type == ITEM_FOOD) {
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

SPELL_FUN(prayer_refresh, sn, level, ch, vo)
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

SPELL_FUN(prayer_restoring_light, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room is already lit with magic light.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("Rest a while, you're tired from previous one.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 25;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act("The room becomes lit with warm light.", ch, NULL, NULL, TO_ROOM);
	act_char("Ok.", ch);
}

SPELL_FUN(prayer_word_of_recall, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *pet;

	if (IS_NPC(victim))
		return;

	if (victim->fighting
	&&  !can_flee(victim)) {
		if (victim == ch)
			act_char("Your honour doesn't let you recall!", ch);
		else {
			act_puts("You can't cast this spell to a honourable fighting $t!",
				 ch, victim->ch_class, NULL,
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
		act_char("Your god doesn't hear you.", ch);
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

SPELL_FUN(prayer_inspire, sn, level, ch, vo)
{
	CHAR_DATA *gch;

	foreach (gch, char_in_room(ch->in_room)) {
		AFFECT_DATA *paf;

		if (!is_same_group(gch, ch))
			continue;

		if (spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch))
				break;
			continue;
		}

		if (is_sn_affected(gch, "bless")) {
			if (gch == ch)
				act_char("You are already inspired.", ch);
			else {
				act("$N is already inspired.",
				    ch, NULL, gch, TO_CHAR);
			}
			continue;
		}

		paf = aff_new(TO_AFFECTS, "bless");
		paf->level	= level;
		paf->duration	= level + 6;

		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= level/12;
		affect_to_char(gch, paf);

		INT(paf->location)= APPLY_SAVING_SPELL;
		paf->modifier	= -level/12;
		affect_to_char(gch, paf);
		aff_free(paf);

		act_char("You feel inspired!", gch);
		if (ch != gch) {
			act("You inspire $N with the Creator's power!",
			    ch, NULL, gch, TO_CHAR);
		}
	} end_foreach(gch);
}

SPELL_FUN(prayer_heal, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn, 100 + level / 10);
}

SPELL_FUN(prayer_master_healing, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	   300 + level + dice(1,40));
}

SPELL_FUN(prayer_group_heal, sn, level, ch, vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (is_same_group(ch, gch)
		&&  !has_spec(gch, "clan_battleragers")) {
			spellfun("master healing", NULL, level, ch, gch);
			spellfun("refresh", NULL, level, ch, gch);
		}
	}
}

SPELL_FUN(prayer_superior_heal, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    170 + level + dice(1, 20));
}

SPELL_FUN(prayer_mass_healing, sn, level, ch, vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if ((IS_NPC(ch) && IS_NPC(gch))
		||  (!IS_NPC(ch) && !IS_NPC(gch))) {
			spellfun("heal", NULL, level, ch, gch);
			spellfun("refresh", NULL, level, ch, gch);
		}
	}
}

SPELL_FUN(prayer_dispel_good, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

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

	inflict_spell_damage(ch, victim, level, sn);
}


SPELL_FUN(prayer_dispel_evil, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

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

	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_cure_light_wounds, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    dice(1, 8) + level / 4 + 5);
}

SPELL_FUN(prayer_cure_serious_wounds, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn,
	    dice(2, 8) + level / 3 + 10);
}

SPELL_FUN(prayer_cure_critical_wounds, sn, level, ch, vo)
{
	focus_positive_energy(ch, (CHAR_DATA *) vo, sn, dice(3, 8) + level);
}

SPELL_FUN(prayer_cure_blindness, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!IS_AFFECTED(victim, AFF_BLIND)) {
		if (victim == ch)
			act_char("You aren't blind.", ch);
		else {
			act("$N doesn't appear to be blinded.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	for (paf = victim->affected; paf != NULL; paf = paf->next) {
		if (paf->where == TO_AFFECTS && paf->bitvector == AFF_BLIND) {
			if (check_dispel(level, victim, paf->type)
			&& !IS_AFFECTED(victim, AFF_BLIND)) {
				act_char("Your vision returns!", victim);
				act("$n is no longer blinded.",
				    victim, NULL, NULL, TO_ROOM);
			} else
				act_char("Spell failed.",ch);

			return;
		}
	}
}

SPELL_FUN(prayer_cure_poison, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_sn_affected(victim, "poison")) {
		if (victim == ch)
			act_char("You aren't poisoned.", ch);
		else {
			act("$N doesn't appear to be poisoned.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (check_dispel(level, victim, "poison")) {
		act_char("A warm feeling runs through your body.", victim);
		act("$n looks much better.", victim, NULL, NULL, TO_ROOM);
	} else
		act_char("Your god doesn't hear you.", ch);
}

/* RT added to cure plague */
SPELL_FUN(prayer_cure_disease, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_sn_affected(victim, "plague")) {
		if (victim == ch)
			act_char("You aren't ill.", ch);
		else {
			act("$N doesn't appear to be diseased.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (check_dispel(level, victim, "plague")) {
		act("$n looks relieved as $s sores vanish.",
		    victim, NULL, NULL, TO_ROOM);
	} else
		act_char("Your god doesn't hear you.", ch);
}

SPELL_FUN(prayer_group_defence, sn, level, ch, vo)
{
	CHAR_DATA *gch;

	foreach (gch, char_in_room(ch->in_room)) {
		if (!is_same_group(gch, ch))
			continue;

		if (spellbane(gch, ch, 100, dice(2, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch))
				break;
			continue;
		}

		if (is_sn_affected(gch, "armor")) {
			if (gch == ch)
				act_char("You are already armored.", ch);
			else {
				act("$N is already armored.",
				    ch, NULL, gch, TO_CHAR);
			}
		}  else {
			AFFECT_DATA *paf;

			paf = aff_new(TO_AFFECTS, "armor");
			paf->level     = level;
			paf->duration  = level;
			INT(paf->location) = APPLY_AC;
			paf->modifier  = -20;
			affect_to_char(gch, paf);
			aff_free(paf);

			act_char("You feel someone protecting you.", gch);
			if (ch != gch) {
				act("$N is protected by your magic.",
				    ch, NULL, gch, TO_CHAR);
			}
		}

		if (is_sn_affected(gch, "shield")) {
			if (gch == ch)
				act_char("You are already shielded.", ch);
			else {
				act("$N is already shielded.",
				    ch, NULL, gch, TO_CHAR);
			}
		} else {
			AFFECT_DATA *paf;

			paf = aff_new(TO_AFFECTS, "shield");
			paf->level	= level;
			paf->duration	= level;
			INT(paf->location)= APPLY_AC;
			paf->modifier	= -20;
			affect_to_char(gch, paf);
			aff_free(paf);

			act_char("You are surrounded by a force shield.", gch);
			if(ch != gch) {
				act("$N is surrounded by a force shield.",
				    ch, NULL, gch, TO_CHAR);
			}
		}
	} end_foreach(gch);
}

SPELL_FUN(prayer_turn, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	CHAR_DATA *vch;

	if (is_sn_affected(ch, sn)) {
		act_char("This power is used too recently.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5;
	affect_to_char(ch, paf);
	aff_free(paf);

	foreach (vch, char_in_room(ch->in_room)) {
		int dam, align;

		if (is_safe_spell(ch, vch, TRUE))
			continue;
		if (IS_EVIL(ch)) {
			vch = ch;
			act_char("The energy explodes inside you!", ch);
		}

		if (vch != ch) {
			act("$n raises $s hand, and a blinding ray of light shoots forth!",
			    ch, NULL, NULL, TO_ROOM);
			act_char("You raise your hand and a blinding ray of light shoots forth!", ch);
		}

		if (IS_GOOD(vch) || IS_NEUTRAL(vch)) {
			act("$n seems unharmed by the light.",
			    vch, NULL, vch, TO_ROOM);
			act_char("The light seems powerless to affect you.", vch);
			continue;
		}

		dam = calc_spell_damage(ch, level, sn);
		if (saves_spell(level, vch, DAM_HOLY))
			dam /= 2;

		align = vch->alignment - 350;
		if (align < -1000)
			align = -1000 + (align + 1000) / 3;

		dam = dam * align * align / 1000000;
		damage(ch, vch, dam, sn, DAM_F_SHOW);
		if (!IS_EXTRACTED(vch)
		&&  !IS_CLAN_GUARD(vch)
		&&  !IS_SET(vch->in_room->room_flags, ROOM_BATTLE_ARENA))
			dofun("flee", vch, str_empty);
	} end_foreach(vch);
}

SPELL_FUN(prayer_mana_restore, sn, level, ch, vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	int restore;

	restore = 5 * level / 2;
	vch->mana = UMIN(vch->max_mana, vch->mana + restore);
	act("A warm glow passes through you.", vch, NULL, NULL, TO_CHAR);
	if (ch != vch) act_char("Ok.", ch);
}

SPELL_FUN(prayer_severity_force, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act_puts("You cracked the ground towards the $N.",
		 ch, NULL, victim, TO_CHAR, POS_DEAD);
	act("$n cracked the ground towards you!", ch, NULL, victim, TO_VICT);
	inflict_spell_damage(ch, victim, level, sn);
}

#define OBJ_VNUM_SPRING			22

SPELL_FUN(prayer_create_spring, sn, level, ch, vo)
{
	OBJ_DATA *spring;

	spring = create_obj(OBJ_VNUM_SPRING, 0);
	if (spring == NULL)
		return;

	spring->timer = level;
	obj_to_room(spring, ch->in_room);
	act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
	act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
}

#define OBJ_VNUM_MUSHROOM		20

SPELL_FUN(prayer_create_food, sn, level, ch, vo)
{
	OBJ_DATA *mushroom;

	mushroom = create_obj(OBJ_VNUM_MUSHROOM, 0);
	if (mushroom == NULL)
		return;

	INT(mushroom->value[0]) = level / 2;
	INT(mushroom->value[1]) = level;
	mushroom->level = level;
	obj_to_room(mushroom, ch->in_room);
	act("$p suddenly appears.", ch, mushroom, NULL, TO_ALL);
}

SPELL_FUN(prayer_create_water, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;

	if (obj->pObjIndex->item_type != ITEM_DRINK_CON) {
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

SPELL_FUN(prayer_resilience, sn, level, ch, vo)
{
	if (!is_sn_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_RESISTS, sn);
		paf->duration	= level / 10;
		paf->level	= level;
		INT(paf->location)= DAM_NEGATIVE;
		paf->modifier	= 45;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_char("You are now resistive to draining attacks.", ch);
	} else
		act_char("You are already resistive to draining attacks.", ch);
}

SPELL_FUN(prayer_mass_sanctuary, sn, level, ch, vo)
{
	CHAR_DATA *gch;

	foreach (gch, char_in_room(ch->in_room)) {
		if (!is_same_group(gch, ch))
			continue;

		if (spellbane(gch, ch, 100, dice(3, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch))
				break;
			continue;
		}

		spellfun("sanctuary", NULL, level, ch, gch);
	} end_foreach(gch);
}

SPELL_FUN(prayer_lightning_bolt, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_sanctify_lands, sn, level, ch, vo)
{
	if (number_bits(1) == 0) {
		act_char("You failed.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, "cursed lands")) {
		affect_strip_room(ch->in_room, "cursed lands");
		act_char("The curse of the land wears off.", ch);
		act("The curse of the land wears off.",
		    ch, NULL, NULL, TO_ROOM);
	}

	if (is_sn_affected_room(ch->in_room, "deadly venom")) {
		affect_strip_room(ch->in_room, "deadly venom");
		act_char("The land seems more healthy.", ch);
		act("The land seems more healthy.", ch, NULL, NULL, TO_ROOM);
	}

	if (is_sn_affected_room(ch->in_room, "mysterious dream")) {
		affect_strip_room(ch->in_room, "mysterious dream");
		act_char("The land wake up from mysterious dream.", ch);
		act("The land wake up from mysterious dream.",
		    ch, NULL, NULL, TO_ROOM);
	}

	if (is_sn_affected_room(ch->in_room, "black death")) {
		act_char("The disease of the land has been treated.", ch);
		act("The disease of the land has been treated.",
		    ch, NULL, NULL, TO_ROOM);
		affect_strip_room(ch->in_room, "black death");
	}

	if (is_sn_affected_room(ch->in_room, "lethargic mist")) {
		act_char("The lethargic mist dissolves.", ch);
		act("The lethargic mist dissolves.", ch, NULL, NULL, TO_ROOM);
		affect_strip_room(ch->in_room, "lethargic mist");
	}

	if (is_sn_affected_room(ch->in_room, "acid fog")) {
		act("Acid fog turnes into steam and dissolves.",
		    ch, NULL, NULL, TO_ALL);
		affect_strip_room(ch->in_room, "acid fog");
	}
}

SPELL_FUN(prayer_cursed_lands, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (IS_AFFECTED(ch->in_room, RAFF_CURSE)) {
		act_char("This room has already been cursed.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->bitvector	= RAFF_CURSE;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The gods has forsaken the room.", ch);
	act("The gods has forsaken the room.", ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_desert_fist, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if ((ch->in_room->sector_type != SECT_HILLS)
	&&  (ch->in_room->sector_type != SECT_MOUNTAIN)
	&&  (ch->in_room->sector_type != SECT_DESERT)) {
		act_char("You don't find any sand here to create a fist.", ch);
		ch->wait = 0;
		return;
	}

	act("An existing parcel of sand rises up and forms a fist and pummels $n.",
	    victim, NULL, NULL, TO_ROOM);
	act("An existing parcel of sand rises up and forms a fist and pummels you.",
	    victim, NULL, NULL, TO_CHAR);
	inflict_spell_damage(ch, victim, level, sn);
}

/* RT calm spell stops all fighting in the room */
SPELL_FUN(prayer_calm, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;
	int chance;

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
			AFFECT_DATA *paf;

			if (IS_SET(vch->form, FORM_UNDEAD)
			||  IS_SET(vch->form, FORM_CONSTRUCT))
				continue;

			if (IS_AFFECTED(vch, AFF_CALM | AFF_BERSERK)
			||  is_sn_affected(vch, "frenzy"))
				continue;

			act_char("A wave of calm passes over you.", vch);
			if (vch->fighting || vch->position == POS_FIGHTING)
				stop_fighting(vch, FALSE);

			paf = aff_new(TO_AFFECTS, sn);
			paf->level = level;
			paf->duration = level/4;
			INT(paf->location) = APPLY_HITROLL;
			if (!IS_NPC(vch))
				paf->modifier = -5;
			else
				paf->modifier = -2;
			paf->bitvector = AFF_CALM;
			affect_to_char(vch, paf);

			INT(paf->location) = APPLY_DAMROLL;
			affect_to_char(vch, paf);
			aff_free(paf);
		}
	}
}

SPELL_FUN(prayer_harm, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	/* Very interesting */
	dam = UMAX( 20, victim->hit - dice(1,4));
	if (saves_spell(level, victim,DAM_HARM))
		dam = UMIN(50, dam / 2);
	dam = UMIN(100, dam);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(prayer_wrath, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;

	if (IS_GOOD(victim)) {
		act("The gods protect $N.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim)) {
		act("$N does not seem to be affected.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	inflict_spell_damage(ch, victim, level, sn);
	if (IS_EXTRACTED(victim))
		return;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_HOLY))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 2*level;
	INT(paf->location) = APPLY_HITROLL;
	paf->modifier	= -1 * (level / 8);
	affect_to_char(victim, paf);

	INT(paf->location) = APPLY_SAVING_SPELL;
	paf->modifier	= level / 8;
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel unclean.", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
}

/*
 * can be called with ch == NULL, vo == ch (e.g.: obj prog of tattoo of venus)
 */
SPELL_FUN(prayer_bless, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,ITEM_BLESS)) {
			act("$p is already blessed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			paf = affect_find(obj->affected, "curse");
			if (!saves_dispel(level,paf != NULL ? paf->level : obj->level, 0)) {
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows a pale blue.",
				    ch, obj, NULL, TO_ALL);
				REMOVE_OBJ_STAT(obj, ITEM_EVIL);
				return;
			} else {
				act("The evil of $p is too powerful for you to overcome.",
				    ch, obj, NULL, TO_CHAR);
				return;
			}
		}

		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= (6 + level / 2);
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= -1;
		paf->bitvector	= ITEM_BLESS;
		affect_to_obj(obj, paf);
		aff_free(paf);

		act("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already blessed.", ch);
		else if (ch) {
			act("$N already has divine favor.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (6 + level / 2);
	INT(paf->location)= APPLY_LUCK;
	paf->modifier	= UMAX(1, level / 8);
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel righteous.", victim);
	if (ch && ch != victim) {
		act("You grant $N the favor of your god.",
		    ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(prayer_inflict_light_wounds, sn, level, ch, vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(1, 8) + level / 3);
}

SPELL_FUN(prayer_inflict_serious_wounds, sn, level, ch, vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(2, 8) + level / 2);
}

SPELL_FUN(prayer_inflict_critical_wounds, sn, level, ch, vo)
{
	focus_negative_energy(ch, (CHAR_DATA *) vo, sn, dice(3, 8) + level - 6);
}

/* RT replacement demonfire spell */
SPELL_FUN(prayer_demonfire, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!IS_NPC(ch) && !IS_EVIL(ch)) {
		victim = ch;
		act_char("The demons turn upon you!", ch);
	}

	if (victim != ch) {
		act("$n calls forth the demons of Hell upon $N!",
		    ch, NULL, victim, TO_NOTVICT);
		act("$n has assailed you with the demons of Hell!",
		    ch, NULL, victim, TO_VICT);
		act_char("You conjure forth the demons of hell!", ch);
	}
	spellfun("curse", NULL, 3 * level / 4, ch, victim);
	inflict_spell_damage(ch, victim, level, sn);
}

/* mental */
SPELL_FUN(prayer_mind_wrack, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("$n stares intently at $N, causing $N to seem very lethargic.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_remove_fear, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "fear"))
	{
	act_char("You feel more brave.", victim);
	act("$n looks more conscious.", victim, NULL, NULL, TO_ROOM);
	}
	else act_char("You failed.", ch);
}

SPELL_FUN(prayer_call_lightning, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	int dam;

	if (!IS_OUTSIDE(ch)) {
		act_char("You must be out of doors.", ch);
		return;
	}

	if (weather_info.sky < SKY_RAINING) {
		act_char("You need bad weather.", ch);
		return;
	}

	dam = calc_spell_damage(ch, level, sn);
	act_char("Gods' lightning strikes your foes!", ch);
	act("$n calls lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);

	foreach (vch, char_in_world()) {
		if (vch->in_room == ch->in_room) {
			if (is_safe_spell(ch, vch, TRUE))
				continue;
			damage(ch, vch,
			       saves_spell(level, vch, DAM_LIGHTNING) ?
			       dam / 2 : dam, sn, DAM_F_SHOW);
			continue;
		}

		if (vch->in_room->area == ch->in_room->area
		&&  IS_OUTSIDE(vch)
		&&  IS_AWAKE(vch))
			act_char("Lightning flashes in the sky.", vch);
	} end_foreach(vch);
}

SPELL_FUN(prayer_protection_good, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||  IS_AFFECTED(victim, AFF_PROTECT_EVIL)) {
		if (victim == ch)
			act_char("You are already protected.", ch);
		else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (10 + level / 5);
	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -(1+level/10);
	paf->bitvector	= AFF_PROTECT_GOOD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel aligned with darkness.", victim);
	if (ch != victim)
		act("$N is protected from good.", ch, NULL, victim, TO_CHAR);
}

SPELL_FUN(prayer_protection_evil, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL)
	||  IS_AFFECTED(victim, AFF_PROTECT_GOOD)) {
		if (victim == ch)
			act_char("You are already protected.", ch);
		else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (10 + level / 5);
	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -(1 + level/10);
	paf->bitvector	= AFF_PROTECT_EVIL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel holy and pure.", victim);
	if (ch != victim)
		act("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
}

SPELL_FUN(prayer_restoration, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act("You do not have enough power.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	victim->hit = victim->max_hit;
	update_pos(victim);
	act("A warm feeling fills your body.", victim, NULL, NULL, TO_CHAR);
	act("A bleeding wounds on $N's body vanish.", NULL, victim, NULL, TO_NOTVICT);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5;
	affect_to_char(ch, paf);
	aff_free(paf);
}

#define OBJ_VNUM_HOLY_HAMMER		18

SPELL_FUN(prayer_holy_hammer, sn, level, ch, vo)
{
	OBJ_DATA *hammer;
	AFFECT_DATA *paf;

	hammer = create_obj(OBJ_VNUM_HOLY_HAMMER, 0);
	if (hammer == NULL)
		return;

	hammer->level = ch->level;
	hammer->timer = level * 3;
	INT(hammer->value[2]) = (level / 10) + 1;

	paf = aff_new(TO_OBJECT, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->modifier	= level/7 +3;

	INT(paf->location)= APPLY_HITROLL;
	affect_to_obj(hammer, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(hammer, paf);
	aff_free(paf);

	act ("You create a Holy Hammer.", ch, NULL, NULL, TO_CHAR);
	act ("$n creates a Holy Hammer.", ch, NULL, NULL, TO_ROOM);
	obj_to_char_check(hammer, ch);
}

SPELL_FUN(prayer_hold_person, sn, level, ch, vo)
{
	CHAR_DATA* victim = (CHAR_DATA*) vo;

	if (saves_spell(level, victim, DAM_OTHER)) {
		act_char("You failed.", ch);
		act("$n tries to hold you, but fails.", ch, victim, NULL, TO_VICT);
		return;
	}

	hold(ch, victim, 1, -level/12, level);
}

SPELL_FUN(prayer_lethargic_mist, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been full of lethargic mist.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The air in the room makes you slowing down.", ch);
	act("The air in the room makes you slowing down.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_mind_wrench, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	act("$n stares intently at $N, causing $N to seem very hyperactive.",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

/* RT really nasty high-level attack spell */
SPELL_FUN(prayer_holy_word, sn, level, ch, vo)
{
	CHAR_DATA *vch;

	act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
	act_char("You utter a word of divine power.", ch);

	foreach (vch, char_in_room(ch->in_room)) {
		if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
		    (IS_EVIL(ch) && IS_EVIL(vch)) ||
		    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
			act_char("You feel full more powerful.", vch);
			spellfun("frenzy", NULL, level, ch, vch);
			spellfun("bless", NULL, level, ch, vch);
			continue;
		}

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if ((IS_GOOD(ch) && IS_EVIL(vch))
		||  (IS_EVIL(ch) && IS_GOOD(vch))) {
			spellfun("curse", NULL, level, ch, vch);
			act_char("You are struck down!", vch);
			inflict_spell_damage(ch, vch, level, sn);
			continue;
		}

		if (IS_NEUTRAL(ch)) {
			spellfun("curse", NULL, level/2, ch, vch);
			act_char("You are struck down!", vch);
			inflict_spell_damage(ch, vch, level/2, sn);
			continue;
		}
	} end_foreach(vch);

	act_puts("You feel drained.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
	if (!IS_NPC(ch))
		gain_exp(ch, -1 * number_range(1, 10) * 5);
	ch->move = ch->move * 3 / 4;
	ch->hit = ch->hit * 3 / 4;
}

SPELL_FUN(prayer_healing_light, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been healed by light.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 10;
	INT(paf->location)= APPLY_ROOM_HEAL;
	paf->modifier	= level * 3 / 2;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with healing light.", ch);
	act("The room starts to be filled with $n's healing light.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_sanctuary, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (!can_cast_sanctuary(ch, victim))
		return;

	if (IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_EVIL(ch) ? 2 : 1),
		       "+divine", DAM_F_SHOW);
		return;
	}

	if (IS_EVIL(victim)) {
		act("Your god does not seems to like $N.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 6;
	paf->bitvector	= AFF_SANCTUARY;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by a white aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

SPELL_FUN(prayer_black_shroud, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA *paf;

	if (!can_cast_sanctuary(ch, victim))
		return;

	if (!IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_GOOD(ch) ? 2 : 1),
		       "+divine", DAM_F_SHOW);
		return;
	}

	if (!IS_EVIL(victim)) {
		act("Your god does not seems to like $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level     = level;
	paf->duration  = level/6;
	paf->bitvector = AFF_BLACK_SHROUD;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n is surrounded by black aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by black aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

/*  Cleric version of astra_walk  */
SPELL_FUN(prayer_solar_flight, sn, level, ch, vo)
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

	teleport_char(ch, NULL, victim->in_room,
		      "$n disappears in a blinding flash of light!",
		      "You dissolve in a blinding flash of light!",
		      "$n appears in a blinding flash of light!");
}

SPELL_FUN(prayer_black_death, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been diseased.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled by disease.", ch);
	act("The room starts to be filled by disease.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_etheral_fist, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",
	    ch, NULL, victim, TO_NOTVICT);
	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_earthquake, sn, level, ch, vo)
{
	ROOM_INDEX_DATA *room;
	EXIT_DATA *to_room, *back;
	CHAR_DATA *vch;
	int dam, door = -1;

	act_char("The earth trembles beneath your feet!", ch);
	act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

	if (!IS_NULLSTR(target_name)) {
		if (!check_blind(ch)
		||  (door = exit_lookup(target_name)) < 0
		||  (to_room = ch->in_room->exit[door]) == NULL
		||  IS_SET(to_room->exit_info, EX_CLOSED)
		||  (room = to_room->to_room.r) == NULL
		||  (back = room->exit[rev_dir[door]]) == NULL
		||  IS_SET(back->exit_info, EX_CLOSED)
		||  back->to_room.r != ch->in_room)
			return;
	} else	
		room = ch->in_room;

	dam = calc_spell_damage(ch, level, sn);

	foreach (vch, char_in_room(room)) {
		if ((IS_NPC(vch) && IS_NPC(ch))
		||  (IS_NPC(vch) && IS_SET(vch->pMobIndex->act, ACT_NOTRACK))
		||  is_safe_spell(ch, vch, TRUE))
			continue;
			
		if (IS_AFFECTED(vch, AFF_FLYING)
		||  saves_spell(level, vch, DAM_EARTH))
			dam = 0;

		damage(ch, vch, dam, sn, DAM_F_SHOW);
		
		if (IS_NPC(vch) 
		&&  room != ch->in_room)
			path_to_track(ch, vch, door);
	} end_foreach(vch);
}

SPELL_FUN(prayer_blade_barrier, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("Many sharp blades appear around $n and crash $N.",
	    ch, NULL, victim, TO_ROOM);
	act("Many sharp blades appear around you and crash $N.",
	    ch, NULL, victim, TO_CHAR);
	act("Many sharp blades appear around $n and crash you!",
	    ch, NULL, victim, TO_VICT);

	dam = dice(level,5);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_F_SHOW);

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 4);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);

	if (number_percent() < 75)
		return;

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 2);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);

	if (number_percent() < 50)
		return;

	act("The blade barriers crash $n!", victim, NULL, NULL, TO_ROOM);
	dam = dice(level, 3);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_F_SHOW);
	act("The blade barriers crash you!", victim, NULL, NULL, TO_CHAR);
}

SPELL_FUN(prayer_anathema, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	int strength;

	strength = (ch->alignment - victim->alignment) / 200 * level / 30;
	if (strength < 0) strength = -strength;
	if (strength < 7) {
		act_puts("Oh, no. Your god seems to like $N.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		act_puts("$N is already cursed.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	level += strength / 6;

	if (saves_spell(level, victim, DAM_HOLY)) {
		act_char("You failed.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (8 + level/10);

	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_LEVEL;
	paf->modifier	= -strength / 7;
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);

	paf->where	= TO_SKILLS;
	paf->location.s	= str_empty;
	paf->modifier	= -strength;
	paf->bitvector	= SK_AFF_ALL;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("$n looks very uncomfortable.", victim, NULL, NULL, TO_ROOM);
	act_char("You feel unclean.", victim);
}

SPELL_FUN(prayer_heat_metal, sn, level, ch, vo)
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
		||  IS_OBJ_STAT(obj_lose, ITEM_BURN_PROOF))
			continue;

		switch (obj_lose->pObjIndex->item_type) {
		case ITEM_ARMOR:
			if (obj_lose->wear_loc == -1) {
				/*
				 * in inventory -- drop it if we can
				 */
				if (can_drop_obj(victim, obj_lose)) {
					act("$n yelps and throws $p to the ground!",
					     victim, obj_lose, NULL, TO_ROOM);
					act("You yelp and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1,obj_lose->level) / 6;
					if (!IS_SET(victim->in_room->room_flags,
						    ROOM_BATTLE_ARENA))
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
				if (!IS_SET(victim->in_room->room_flags,
					    ROOM_BATTLE_ARENA))
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
					act("You yelp and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1, obj_lose->level) / 6;
					if (!IS_SET(victim->in_room->room_flags,
						    ROOM_BATTLE_ARENA))
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
				if (!IS_SET(victim->in_room->room_flags,
					    ROOM_BATTLE_ARENA))
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
		damage(ch, victim, dam, sn, DAM_F_SHOW);
	}
}

SPELL_FUN(prayer_mind_light, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already had booster of mana.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= level / 30;
	INT(paf->location)= APPLY_ROOM_MANA;
	paf->modifier	= level * 3 / 2;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled with mind light.", ch);
	act("The room starts to be filled with $n's mind light.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_free_action, sn, level, ch, vo)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act("Your movements are already free.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_SKILLS, "free action");
	paf->level	= level;
	paf->duration	= level / 12;
	paf->location.s	= str_dup("swimming");
	paf->modifier	= 100;
	paf->bitvector	= SK_AFF_TEACH;
	affect_to_char(vch, paf);
	aff_free(paf);

	act("You can move easier.", vch, NULL, NULL, TO_CHAR);
	if (ch != vch)
		act("You help $N to move easier.", ch, NULL, vch, TO_CHAR);
}

SPELL_FUN(prayer_deadly_venom, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been affected by deadly venom.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("The room starts to be filled by poison.", ch);
	act("The room starts to be filled by poison.", ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_ray_of_truth, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch)) {
		victim = ch;
		act_char("The energy explodes inside you!", ch);
	}

	if (victim != ch) {
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    ch, NULL, NULL, TO_ROOM);
		act_char("You raise your hand and a blinding ray of light shoots forth!", ch);
	}

	if (IS_GOOD(victim)) {
		act("$n seems unharmed by the light.",
		    victim, NULL, victim, TO_ROOM);
		act_char("The light seems powerless to affect you.", victim);
		return;
	}

	dam = calc_spell_damage(ch, level, sn);
	if (saves_spell(level, victim, DAM_HOLY))
		dam /= 2;

	align = victim->alignment;
	align -= 350;

	if (align < -1000)
		align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;

	spellfun("blindness", NULL, 4 * level / 3, ch, victim);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(prayer_aid, sn, level, ch, vo)
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

	victim->hit += level * 5;
	update_pos(victim);
	act_char("A warm feeling fills your body.", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act_char("Ok.", ch);
}

/* added by chronos */
SPELL_FUN(prayer_bluefire, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!IS_NPC(ch) && !IS_NEUTRAL(ch)) {
		victim = ch;
		act_char("Your blue fire turn upon you!", ch);
	}

	if (victim != ch) {
		act("$n calls forth the blue fire of earth $N!",
		    ch, NULL, victim, TO_ROOM);
		act("$n has assailed you with the neutrals of earth!",
		    ch, NULL, victim, TO_VICT);
		act_char("You conjure forth the blue fire!", ch);
	}

	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_bless_weapon, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if (obj->pObjIndex->item_type != ITEM_WEAPON) {
		act_char("That isn't a weapon.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be blessed.", ch);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)) {
		act("$p is already blessed for holy attacks.",
		    ch, obj, NULL, TO_CHAR);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
	||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
		act("You can't seem to bless $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_WEAPON, sn);
	paf->level	= level / 2;
	paf->duration	= level / 8;
	paf->bitvector	= WEAPON_HOLY;
	affect_to_obj(obj, paf);
	aff_free(paf);

	act("$p is prepared for holy attacks.",ch,obj,NULL,TO_ALL);
}

SPELL_FUN(prayer_control_weather, sn, level, ch, vo)
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

SPELL_FUN(prayer_benediction, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	int strength = 0;

	if (is_sn_affected(victim, sn)) {
		if (victim == ch) {
			act("You are already blessed.",
				ch, NULL, NULL, TO_CHAR);
		} else {
			act("$N is already blessed.",
				ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_EVIL(victim))
		strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
	if (IS_GOOD(victim))
		strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
	if (IS_NEUTRAL(victim))
		strength = IS_NEUTRAL(ch) ? 2 : 1;
	if (!strength) {
		act("Your god does not seems to like $N.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 5 + level / 2;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= level / 8 * strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= 0 - level / 8 * strength;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_LEVEL;
	paf->modifier	= strength;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("You feel righteous.", victim, NULL, NULL, TO_CHAR);
	if (victim != ch) {
		act("You grant $N favor of your god.",
			ch, NULL, victim, TO_CHAR);
	}
}

SPELL_FUN(prayer_curse, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			act("$p is already filled with evil.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
			int spell_level;

			paf = affect_find(obj->affected, "bless");
			spell_level = paf != NULL ? paf->level : obj->level;

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

		paf = aff_new(TO_OBJECT, sn);
		paf->level	= level;
		paf->duration	= (8 + level / 5);
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= 1;
		paf->bitvector	= ITEM_EVIL;
		affect_to_obj(obj, paf);
		aff_free(paf);

		act("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character curses */
	victim = (CHAR_DATA *) vo;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_NEGATIVE))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= (8 + level / 10);
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -(level / 8 + 1);
	paf->bitvector	= AFF_CURSE;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= level / 8 + 1;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You feel unclean.", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
}

SPELL_FUN(prayer_remove_curse, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	bool found = FALSE;
	OBJ_DATA *obj;

	/* do object cases first */
	if (mem_is(vo, MT_OBJ)) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_NODROP)
		||  IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
			if (!OBJ_IS(obj, OBJ_NOUNCURSE)
			&&  !saves_dispel(level + 2, obj->level, 0)) {
				REMOVE_OBJ_STAT(
				    obj, ITEM_NODROP | ITEM_NOREMOVE);
				act("$p glows blue.", ch, obj, NULL, TO_ALL);
				return;
			}

			act("The curse on $p is beyond your power.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		act_char("Nothing happens...", ch);
		return;
	}

	/* characters */
	victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "curse")) {
		act_char("You feel better.", victim);
		act("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
	}

	for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content) {
		if ((IS_OBJ_STAT(obj, ITEM_NODROP) ||
		     IS_OBJ_STAT(obj, ITEM_NOREMOVE))
		&&  !OBJ_IS(obj, OBJ_NOUNCURSE)
		&&  !saves_dispel(level, obj->level, 0)) {
			found = TRUE;
			REMOVE_OBJ_STAT(obj, ITEM_NODROP | ITEM_NOREMOVE);
			act("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
			act("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
		}
	}
}

SPELL_FUN(prayer_flamestrike, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	inflict_spell_damage(ch, victim, level, sn);
}

SPELL_FUN(prayer_know_alignment, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	const char *msg;

	if (IS_GOOD(victim))
		msg = "$N has a pure and good aura.";
	else if (IS_NEUTRAL(victim))
		msg = "$N act as no align.";
	else
		msg = "$N is the embodiment of pure evil!";
	act(msg, ch, NULL, victim, TO_CHAR);

	if (!IS_NPC(victim)) {
		switch (PC(victim)->ethos) {
		case ETHOS_LAWFUL:
			msg = "$N upholds the laws.";
			break;
		case ETHOS_NEUTRAL:
			msg = "$N seems ambivalent to society.";
			break;
		case ETHOS_CHAOTIC:
			msg = "$N seems very chaotic.";
			break;
		default:
			msg = "$N doesn't know where they stand on the laws.";
			break;
		}
		act(msg, ch, NULL, victim, TO_CHAR);
	}
}

/* RT clerical berserking spell */
SPELL_FUN(prayer_frenzy, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)) {
		if (victim == ch)
			act_char("You are already in a frenzy.", ch);
		else
			act("$N is already in a frenzy.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_sn_affected(victim, "calm")) {
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

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= UMAX(2, level / 3);
	paf->modifier	= UMAX(1, level / 6);

	INT(paf->location)= APPLY_HITROLL;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_AC;
	paf->modifier	= 5 * UMAX(1, level / 6);
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You are filled with holy wrath!", victim);
	act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_awaken, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->position != POS_SLEEPING) {
		if (victim != ch)
			act("But $N isn't sleeping!", ch, NULL, victim,
			    TO_CHAR);
		else
			act_char("You are already awake, aren't you?", ch);

		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP)) {
		REMOVE_BIT(victim->affected_by, AFF_SLEEP);
		affect_bit_strip(victim, TO_AFFECTS, AFF_SLEEP);
	}

	dofun("wake", ch, victim->name);
}

SPELL_FUN(prayer_shapechange, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	char arg[MAX_INPUT_LENGTH];
	int req_level;
	const char *form_name;

	if (is_sn_affected(ch, sn) || ch->shapeform) {
		act_char("You must return to your natural form first.", ch);
		return;
	}

	if (IS_NULLSTR(target_name)) {
		act_char("How do you wish to change your shape?", ch);
		return;
	}

	target_name = one_argument(target_name, arg, sizeof(arg));
	if (!str_cmp(arg, "wolf")) {
		req_level = 0;
		form_name = "wolf";
	} else if (!str_cmp(arg, "bear")) {
		req_level = MAX_LEVEL * 3 / 5;
		form_name = "bear";
	} else {
		act_char("You do not know how to change your shape to that.",
		    ch);
		return;
	}

	if (ch->level < req_level) {
		act_char("You are too unexpierenced.", ch);
		return;
	}

	paf = aff_new(TO_FORM, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->location.s	= str_dup(form_name);
	affect_to_char(ch, paf);
	aff_free(paf);
}

SPELL_FUN(prayer_hold_animal, sn, level, ch, vo)
{
	CHAR_DATA* victim = (CHAR_DATA*) vo;

	if (!IS_SET(victim->form, FORM_ANIMAL)) {
		if (victim == ch)
			act_char("Do you really feel as if you are animal?",
			    ch);
		else
			act("$N doesn't seem to be animal.", ch, NULL, victim,
			    TO_CHAR);
		return;
	}

	if (saves_spell(level + 2, victim, DAM_OTHER)) {
		act_char("You failed.", ch);
		act("$n tries to hold you, but fails.", ch, victim, NULL,
		    TO_VICT);
		return;
	}

	hold(ch, victim, 1, -level/12, level);
}

SPELL_FUN(prayer_light, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_AFFECTED(ch->in_room, RAFF_DARKNESS)) {
		affect_bit_strip_room(ch->in_room, RAFF_DARKNESS);
		act("Room seems to be less dark now.", ch, NULL,
		    NULL, TO_ALL);
		return;
	}

	if (IS_AFFECTED(ch->in_room, RAFF_LIGHT)) {
		act("You cannot add more light to this room.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (room_is_dark(ch->in_room)) {
		act("The room is lit by a magical light.",
			ch, NULL, NULL, TO_ALL);
	} else {
		act("Light seems to be somewhat better now.",
			ch, NULL, NULL, TO_ALL);
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 3;
	paf->bitvector	= RAFF_LIGHT;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);
}

SPELL_FUN(prayer_darkness, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_AFFECTED(ch->in_room, RAFF_LIGHT)) {
		affect_bit_strip_room(ch->in_room, RAFF_LIGHT);
		act("Room seems to be less lit now.", ch, NULL,
		    NULL, TO_ALL);
		return;
	}

	if (IS_AFFECTED(ch->in_room, RAFF_DARKNESS)) {
		act_char("It's already pitch-dark here.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 3;
	paf->bitvector	= RAFF_DARKNESS;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act("The darkness falls.", ch, NULL, NULL, TO_ALL);
}

static int air_elemental_stats[MAX_STAT] = {
	23, 23, 3, 24, 16, 25
};

SPELL_FUN(prayer_air_elemental, sn, level, ch, vo)
{
	CHAR_DATA *elemental;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_puts("You cannot summon air elemental right now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (ch->in_room->sector_type == SECT_UNDERWATER) {
		act_char("There is no air here.", ch);
		return;
	}

	if (!can_summon_creature(ch))
		return;

	act("You notice some strange motions in air around you.", ch,
	    NULL, NULL, TO_ALL);

	elemental = create_mob(MOB_VNUM_AIR_ELEMENTAL, 0);
	if (elemental == NULL) {
		act("But nothing else happens.", ch, NULL, NULL, TO_ALL);
		return;
	}

	for (i = 0; i < MAX_STAT; i++) {
		elemental->perm_stat[i] = air_elemental_stats[i];
	}

	SET_HIT(elemental, ch->perm_hit);
	SET_MANA(elemental, ch->max_mana);
	elemental->level = level;
	for (i = 0; i < 4; i++)
		elemental->armor[i] = interpolate(elemental->level, 100, -100);
	elemental->gold = elemental->silver = 0;
	NPC(elemental)->dam.dice_number = number_range(level/15, level/10);
	NPC(elemental)->dam.dice_type   = number_range(level/3, level/2);
	elemental->damroll  = 0;

	act("Air around you concentrates and $N appears.",
		 ch, NULL, elemental, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 48;
	affect_to_char(ch, paf);
	aff_free(paf);

	elemental->master = elemental->leader = ch;

	char_to_room(elemental, ch->in_room);
}

#define MOB_VNUM_THORN_WALL_EAST	32
#define MOB_VNUM_THORN_WALL_WEST	33
#define MOB_VNUM_THORN_WALL_NORTH	34
#define MOB_VNUM_THORN_WALL_SOUTH	35

SPELL_FUN(prayer_wall_of_thorns, sn, level, ch, vo)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	CHAR_DATA *wall;
	AFFECT_DATA *paf;
	int i;
	int vnum;

	if (is_sn_affected(ch, sn)) {
		act_puts("You cannot grow a wall of thorns yet.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((ch->in_room->sector_type != SECT_FIELD &&
	     ch->in_room->sector_type != SECT_FOREST &&
	     ch->in_room->sector_type != SECT_HILLS &&
	     ch->in_room->sector_type != SECT_MOUNTAIN)
	||  IS_SET(ch->in_room->room_flags,
		   ROOM_NOMOB | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)) {
		act_char("You cannot grow a wall of thorns here.", ch);
		return;
	}

	if (IS_NULLSTR(target_name)) {
		act_char("Where do you want to place a wall?", ch);
		return;
	}

	target_name = one_argument(target_name, arg, sizeof(arg));

	if (!str_cmp(arg, "east"))
		vnum = MOB_VNUM_THORN_WALL_EAST;
	else if (!str_cmp(arg, "west"))
		vnum = MOB_VNUM_THORN_WALL_WEST;
	else if (!str_cmp(arg, "north"))
		vnum = MOB_VNUM_THORN_WALL_NORTH;
	else if (!str_cmp(arg, "south"))
		vnum = MOB_VNUM_THORN_WALL_SOUTH;
	else {
		act_char("You cannot place a wall there.", ch);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (IS_NPC(vch) && vch->pMobIndex->vnum == vnum) {
			act("But $N is already grown.", ch, NULL, vch, TO_CHAR);
			return;
		}
	}

	wall = create_mob(vnum, 0);
	if (wall == NULL)
		return;

	SET_HIT(wall, number_range(level * 15, ch->level * 35));
	SET_MANA(wall, ch->max_mana);
	wall->level = level;
	for (i = 0; i < 4; i++)
		wall->armor[i] = 100;
	wall->gold = wall->silver = 0;

	act("Suddenly a $N grows from the ground.", ch, NULL, wall, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 24;
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(wall, ch->in_room);
}

SPELL_FUN(prayer_obscuring_mist, sn, level, ch, vo)
{
	AFFECT_DATA *paf;
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!can_cast_sanctuary(ch, victim))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 6 + 2;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Suddenly obscuring mist surrounds you.", victim);
	act_char("You feel very safe under it's cover.", victim);

	act("$n is surrounded by some kind of mist.", victim, NULL, NULL,
	    TO_ROOM);
}

SPELL_FUN(prayer_treeform, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (ch->shapeform) {
		act_char("You must return to your natural form first.", ch);
		return;
	}

	paf = aff_new(TO_FORM, sn);
	paf->level	= level;
	paf->duration	= -1;
	paf->location.s	= str_dup("tree");
	affect_to_char(ch, paf);
	aff_free(paf);

	act("You are suddenly transformed into $n.", ch, NULL, NULL,
	    TO_CHAR);
}

static void
hold(CHAR_DATA *ch, CHAR_DATA *victim, int duration, int dex_modifier,
     int level)
{
	AFFECT_DATA *paf;

	affect_strip(victim, "hold");

	paf = aff_new(TO_AFFECTS, "hold");
	paf->level		= level;
	paf->duration		= duration;
	paf->owner		= ch;
	paf->modifier		= dex_modifier;
	INT(paf->location)	= APPLY_DEX;

	affect_to_char(victim, paf);

	aff_free(paf);
	act("$N suddenly seems immobile.", ch, NULL, victim, TO_NOTVICT);
	act_char("You are unable to move.", victim);
}

struct mutually_exclusive_spells {
	const char *sn;
	const char *self_cast_msg;
	const char *other_cast_msg;
};

static struct mutually_exclusive_spells mx_sancs[] = {
	/* sanctuary and black shroud should be first */
	{ "sanctuary",		"But you are in sanctuary.",
		"But $N is in sanctuary." },
	{ "black shroud",	"But you are protected by black shroud.",
		"But $N is protected by black shroud." },
	{ "obscuring mist",	"But mist around you already protects you well.",
		"Obscuring mist around $N already protects $gN{him} well enough." },
	{ "fire sphere",	"But hot fire sphere protects you well enough.",
		"Hot fire sphere around $N protects $gN{him} well enough."},
	{ "death ward",		"Death ward already protects you.",
		"Death ward already protects $N."},
	{ "ice sphere",		"But sparkling ice sphere already protects you well.",
		"Sparkling ice sphere around $N protects $gN{him} well enought."},
	{ NULL,	NULL, NULL }
};

static bool
can_cast_sanctuary(CHAR_DATA *ch, CHAR_DATA *victim)
{
	struct mutually_exclusive_spells *p = mx_sancs;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (ch == victim) {
			act_char(mx_sancs[0].self_cast_msg, ch);
			return FALSE;
		} else {
			act(mx_sancs[0].other_cast_msg, ch, NULL, victim,
			    TO_CHAR);
			return FALSE;
		}
	}

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
		if (ch == victim) {
			act_char(mx_sancs[1].self_cast_msg, ch);
			return FALSE;
		} else {
			act(mx_sancs[1].other_cast_msg, ch, NULL, victim,
			    TO_CHAR);
			return FALSE;
		}
	}

	for (; p->sn; p++) {
		if (is_sn_affected(victim, p->sn)) {
			if (ch == victim)
				act_char(p->self_cast_msg, ch);
			else {
				act(p->other_cast_msg, ch, NULL, victim,
				    TO_CHAR);
			}
			return FALSE;
		}
	}

	return TRUE;
}

SPELL_FUN(prayer_fly, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_FLYING)
	||  is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already have flying ability.", ch);
		else {
			act("$N doesn't need your help to fly.",
			    ch,NULL,victim,TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level;
	paf->bitvector	= AFF_FLYING;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("You start to fly.", victim);
	act("$n starts to fly.", victim, NULL, NULL, TO_ROOM);
}

static void
mist_walk(CHAR_DATA *ch, CHAR_DATA *victim)
{
	teleport_char(ch, NULL, victim->in_room,
		      "$n dissolves into a cloud of glowing mist, "
		      "then vanishes!",
		      "You dissolve into a cloud of glowing mist, then "
		      "flow to your target.",
		      "A cloud of glowing mist engulfs you, then withdraws "
		      "to unveil $n!");
}

SPELL_FUN(prayer_mist_walk, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You cann't reach your target.", ch);
		return;
	}

	pet = GET_PET(ch);
	if (pet && pet->in_room != ch->in_room)
		pet = NULL;

	mist_walk(ch, victim);
	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			dofun("stand", pet, str_empty);
		mist_walk(ch, victim);
	}
}

SPELL_FUN(prayer_air_walk, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (IS_AFFECTED(victim, AFF_FLYING)
	||  is_sn_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already stand on the air.", ch);
		else {
			act("$N already stands on the air.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level;
	paf->bitvector	= AFF_FLYING;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Air became solid and you now can stand on it.", victim);
	act("Impossible! $n now stands on the air!", victim, NULL, NULL,
	    TO_ROOM);
}

SPELL_FUN(prayer_nightmare, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (ch == victim)
		act_char("Even you is not so stupid.", ch);

	if (IS_AFFECTED(victim, AFF_SLEEP)
	||  IS_SET(victim->form, FORM_UNDEAD)
	||  IS_SET(victim->form, FORM_CONSTRUCT)
	||  saves_spell(level, victim, DAM_CHARM))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 1 + level/10;
	paf->bitvector	= AFF_SLEEP;
	affect_to_char(victim, paf);
	aff_free(paf);

	if (IS_AWAKE(victim)) {
		act_char("Fear! Nightmares come to you.", victim);
		act("$n puts $N into nightmares.", ch, NULL, victim,
		    TO_NOTVICT);
		act("You put $N into nightmares.", ch, NULL, victim, TO_CHAR);
		victim->position = POS_SLEEPING;
	}
}

SPELL_FUN(prayer_abolish_undead, sn, level, ch, vo)
{
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        CHAR_DATA *tmp_ch;
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;
        int i, dam = 0;
        OBJ_DATA *tattoo, *clanmark;
        PC_DATA *vpc;

	if  (IS_EVIL(ch)) {
                act("You are not of the Light.", ch, NULL, victim, TO_CHAR);
                return;
	}

        if (!IS_SET(victim->form, FORM_UNDEAD) && !IS_VAMPIRE(victim)) {
                act("$N doesn't seem to be an undead.",
		    ch, NULL, victim, TO_CHAR);
                return;
        }

	if (saves_spell(level + 2, victim, DAM_HOLY)
	||  dice_wlb(1, 3, victim, NULL) == 1
	||  IS_IMMORTAL(victim)
	||  IS_CLAN_GUARD(victim)
	||  IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		dam = calc_spell_damage(ch, level, sn);
		damage(ch, victim, dam, sn, DAM_F_SHOW);
		return;
	}

        act("$n's holy light burns you! Your flesh decays into dust.",
	    ch, NULL, victim, TO_VICT);
        act("$n's holy light burns $N! $lu{$N}'s flesh decays into dust.",
	    ch, NULL, victim, TO_NOTVICT);
        act("Your holy light burns $N! $lu{$N}'s flesh decays into dust.",
	    ch, NULL, victim, TO_CHAR);
        act_char("You have been KILLED!", victim);

	act("$N does not exist anymore!", ch, NULL, victim, TO_NOTVICT);

        act_char("You turn into an invincible ghost for a few minutes.",
                     victim);
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
        victim->position	= POS_RESTING;
        victim->hit		= 1;
        victim->mana		= 1;

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

SPELL_FUN(prayer_golden_aura, sn, level, ch, vo)
{
	CHAR_DATA *vch = vo;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		AFFECT_DATA *paf;

		if (!is_same_group(vch, ch) || !IS_GOOD(ch))
			continue;

		if (is_sn_affected(vch, sn)) {
			if (vch == ch)
				act_char("You are already protected by a golden aura.", ch);
			else
				act("$N is already protected by a golden aura.",
				    ch, NULL, vch, TO_CHAR);
			continue;
		}

		paf = aff_new(TO_AFFECTS, sn);
		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= UMAX(1, level / 8);
		paf->bitvector	= 0;
		paf->duration	= level / 4;
		affect_to_char(vch, paf);

		INT(paf->location)= APPLY_SAVING_SPELL;
		paf->modifier	=  -UMAX(1, level / 8);
		affect_to_char(vch, paf);
		aff_free(paf);

		act_char("You feel a golden aura around you.", vch);
		if (ch != vch)
			act("A golden aura surrounds $N.",
			    ch, NULL, vch, TO_CHAR);
	}
}

SPELL_FUN(prayer_fire_sphere, sn, level, ch, vo)
{
        AFFECT_DATA *paf;

        if (is_sn_affected(ch, sn)) {
                act_puts("Fire sphere already surrounds you.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
        };

	if (!can_cast_sanctuary(ch, ch))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->type      = sn;
	paf->level     = level;
	paf->duration  = UMAX (2, level/4);
	paf->modifier  = 0;
	paf->bitvector = 0;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Hot violent fire flashes around you.", ch, NULL, NULL, TO_CHAR);
	act("Hot violent fire flashes around $n.", ch, NULL, NULL, TO_ROOM);
}
/* Domain: sun
 * inflict damage
 * can be used only in the daytime
 */
SPELL_FUN(prayer_sunburst, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	int i = 1, dam;

	if (ch == victim) {
		act_char("You are not so stupid.", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_INSIDE) {
		act_char("You need sun light to use this ability.", ch);
		return;
	}

	switch (weather_info.sunlight) {
	case SUN_DARK:
		act_char("You need sun light to use this ability.", ch);
		return;
	case SUN_RISE:
	case SUN_SET:
		i = 2;
		break;
	case SUN_LIGHT:
		break;
	default:
		act_char("You need sun light to use this ability.", ch);
		return;
	}

	dam = calc_spell_damage(ch, level, sn) / i;
	damage(ch, victim, dam, sn, DAM_F_SHOW);

	if (IS_AFFECTED(victim, AFF_BLIND)
	||  saves_spell(level, victim, DAM_LIGHT))  {
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= -4;
	paf->duration	= 1 + level / 25;
	paf->bitvector	= AFF_BLIND;
	affect_to_char(victim, paf);
	aff_free(paf);

	act_char("Sun light bursts your eyes. You are blind!", victim);
	act("Sun light bursts $N eyes. $gN{He} is blind!",
	    ch, NULL, victim, TO_NOTVICT);
}
/* Domain: air
 * inflict damage to target, area attack with cold effect
 */
SPELL_FUN(prayer_cone_of_cold, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	int dam;

	act("$n creates a freezing blast of air!", ch, NULL, NULL, TO_ROOM);
	act_char("You draw heat from the room to create a blast of freezing air!", ch);
	dam = calc_spell_damage(ch, level, sn) * 4 / 5;
	foreach (vch, char_in_room(ch->in_room)) {
		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (saves_spell(level, vch, DAM_COLD))
			dam /= 2;
		damage(ch, vch, dam, sn, DAM_F_SHOW);
	} end_foreach(vch);
}

SPELL_FUN(prayer_power_word_fear, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;
	bool bad_fail, utter_fail;
	int range;

	bad_fail = utter_fail = FALSE;

	if (victim == ch)
		act_char("That wouldn't work.", ch);

	if (IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("You cann't use this power here.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_char("You is not powerful enough.", ch);
		return;
	}

	act("$n points at $N and utters the word 'Fear!'",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n points at you and utters the word 'Fear!'",
	    ch, NULL, victim, TO_VICT);
	act("You point at $N and utter the word 'Fear!'",
	    ch, NULL, victim, TO_CHAR);

	if (!IS_AWAKE(victim)) {
		act("$n shivers momentarily but it passes.",
		    victim, NULL, NULL, TO_ROOM);
		act_char("You feel a brief terror, but it passes away "
			 "in your dreams.", victim);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		act("$N is already affected by a word of power.",
		    ch, NULL, victim, TO_CHAR);
		act_char("You feel a shiver pass through you but it has "
			 "no further affect.", victim);
		return;
	}

	if (IS_NPC(victim)
	&&  IS_CLAN_GUARD(victim)) {
		act("$N shivers momentarily but it passes.",
		    ch, NULL, NULL, TO_NOTVICT);
		return;
	}

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act("$n shivers momentarily but it passes.",
		    victim, NULL, NULL, TO_ROOM);
		act("You feel a brief terror, but it passes.",
		    victim, NULL, NULL, TO_CHAR);
		return;
	}

	if (!saves_spell(level - 2, victim, DAM_NEGATIVE)) {
		bad_fail = TRUE;
		if (!saves_spell(level - 5, victim, DAM_NEGATIVE))
		utter_fail = TRUE;
	}

	if (utter_fail) {
		act("$n's eyes widen and $gn{his} heart ruptures from shock!",
		    victim, NULL, NULL, TO_ROOM);
		act("You feel a terror so intense your heart stops dead!",
		    victim, NULL, NULL, TO_CHAR);
		raw_kill(ch, victim);
		return;
	}

	act("$n's eyes widen in shock and $gn{his} entire body freezes in "
	    "momentary terror.", victim, NULL, NULL, TO_ROOM);
	act("You feel an overwhelming terror and you shudder in momentary "
	    "shock.", victim, NULL, NULL, TO_CHAR);

	range = UMAX(1, level / 10);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= number_range(1, 5) + range;

	INT(paf->location)= APPLY_CON;
	paf->modifier	= - number_range (2, range + 1);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_STR;
	paf->modifier	= - number_range (2, range + 1);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DEX;
	paf->modifier	= - number_range (1, range);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= - 2 * number_range (2, range + 1);
	affect_to_char(victim, paf);

	INT(paf->location)= APPLY_DAMROLL;
	paf->modifier	= - 2 * number_range (2, range + 1);
	affect_to_char(victim, paf);
	aff_free(paf);

	if (victim->position == POS_FIGHTING)
		dofun("flee", victim, str_empty);

	if (victim->position == POS_FIGHTING)
		dofun("flee", victim, str_empty);

	if (victim->position == POS_FIGHTING)
		dofun("flee", victim, str_empty);

	if (bad_fail)
		WAIT_STATE(ch, 3 * get_pulse("violence"));

}
/* Domain: air
 * area attack, have a chance to blind,
 * falls flying creatures to the ground
 */
SPELL_FUN(prayer_windwall, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	int dam = calc_spell_damage(ch, level, sn);

	act_char("You raise a violent wall of wind to strike your foes.", ch);
	act("$n raises a violent wall of wind, sending debri flying!",
	    ch, NULL, NULL, TO_ROOM);

	foreach (vch, char_in_room(ch->in_room)) {
		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (number_bits(1) == 0
		&&  !saves_spell(level, vch, DAM_AIR)) {
			AFFECT_DATA *paf;

			act("$n appears blinded by the debris.",
			    vch, NULL, NULL, TO_ROOM);

			paf = aff_new(TO_AFFECTS, sn);
			paf->level	= level;
			paf->duration	= 1;
			INT(paf->location)= APPLY_HITROLL;
			paf->modifier	= -3;
			paf->bitvector	= AFF_BLIND;

			affect_join(vch, paf);
			aff_free(paf);
		}

		damage(ch, vch, dam, sn, DAM_F_SHOW);

		if (!IS_AFFECTED(vch, AFF_FLYING)
		||  saves_spell(level, vch, DAM_AIR))
			continue;

		act("$n is thrown wildly to the ground by the air blast!",
		    vch, NULL, NULL, TO_ROOM);
		act_char("You are thrown down by the air blast!", vch);
		REMOVE_BIT(vch->affected_by, AFF_FLYING);
	} end_foreach(vch);
}

/* domain: death
 * This prayer force to sound dreadful wail of the banshee.
 * Room attack with target which gets most of the damage.
 * Sometimes wail of the banshee can cause instant death of
 * the victim. Deafen and undead are unaffected.
 */
SPELL_FUN(prayer_wail_of_the_banshee, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch;
	AFFECT_DATA *paf;
	int dam;

	if (is_sn_affected(ch, sn)) {
		act_char("You can't use this power so soon.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Are you crazy?", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* little timer for priest */
	paf = aff_new(TO_AFFECTS, sn);
	paf->type      = sn;
	paf->duration  = 1 + number_bits(1);
	paf->level     = level;
	paf->modifier  = 0;
	paf->bitvector = 0;
	affect_to_char(ch, paf);
	aff_free(paf);

	/* deafen and undead creatures are unaffected */
	if (is_sn_affected(victim, "deafen")
	||  IS_SET(victim->form, FORM_UNDEAD)) {
		act_char("Your victim ignores all you attempts.", ch);
		act("You looks at $N and don't understand what $gN{he} "
		    "trying to do.", victim, NULL, ch, TO_CHAR);
		return;
	}

	/* insty or damage target */
	dam = calc_spell_damage(ch, level, sn);

	if (!saves_spell(level + 2, victim, DAM_SOUND)
	&&  !dice_wlb(1, 3, victim, NULL) == 1
	&&  !IS_IMMORTAL(victim)
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("Dreadful wail of the banshee fills you"
			 " making you dead.", victim);
		act("Dreadful wail of the banshee fills your "
		    "victim and $gN{his} life is torn.",
		    ch, NULL, victim, TO_CHAR);
		act("Dreadful wail of the banshee fills $N and $gN{his} "
		    "life is torn.", ch, NULL, victim, TO_ROOM);
		handle_death(ch, victim);
	} else
		damage(ch, victim, dam, sn, DAM_F_SHOW);

	/* damage all in the room, ignore deafen and undead */
	dam = dam / 5;
	foreach (vch, char_in_room(ch->in_room)) {
		if (vch == victim
		||  is_sn_affected(vch, "deafen")
		||  IS_SET(vch->form, FORM_UNDEAD)
		||  is_safe_spell(ch, vch, TRUE))
			continue;
		damage(ch, vch, dam, sn, DAM_F_SHOW);
	} end_foreach(vch);
}
/* domain: sun
 * This prayer inform priest about all invisible person in room.
 * Prayer do not make them visible and request sunlight for work.
 */
SPELL_FUN(prayer_sunbeam, sn, level, ch, vo)
{
	CHAR_DATA *vch;
	int i = 0;

	if (IS_AFFECTED(ch, AFF_BLIND)) {
		act_char("You are blind and can't see any shadows.", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_INSIDE) {
		act_char("You need sun light to use this ability.", ch);
		return;
	}

	switch (weather_info.sunlight) {
	case SUN_DARK:
		act_char("Try to use this prayer in the morning.", ch);
		return;
	case SUN_RISE:
	case SUN_SET:
	case SUN_LIGHT:
		break;
	default:
		act_char("You need sun light to use this ability.", ch);
		return;
	}

	act("Sun lights all around you!", ch, NULL, NULL, TO_CHAR);
	act("Sun lights all around!", ch, NULL, NULL, TO_ROOM);

	foreach (vch, char_in_room(ch->in_room)) {
		if (IS_IMMORTAL(vch))
			continue;
		if (can_see(ch, vch)
		||  !saves_spell(level, vch, DAM_LIGHT)) {
			act("    You notice $N's shadow besides you.",
			    ch, NULL, vch, TO_CHAR | ACT_NOCANSEE);
			act("    You see $N's shadow besides $n.",
			    ch, NULL, vch, TO_NOTVICT | ACT_NOCANSEE);
			act("    You see your own shadow besides $n.",
			    ch, NULL, vch, TO_VICT | ACT_NOCANSEE);
			i++;
		} else {
			act_char("    You notice dark shadow besides you.", ch);
			act("    You see dark shadow besides $n.",
			    ch, NULL, NULL, TO_ROOM);
			i++;
		}
	} end_foreach(vch);

	if (i == 0)
		act("    No shadows here.", ch, NULL, NULL, TO_ALL);
}
/* domain: water
 * Prayer filles room with acid fog. Acid damages all creatures
 * in room (owner has immunity), also can corrode items.
 * Enter room event: event_enter_acid_fog
 * Update room event: event_update_acid_fog
 */
SPELL_FUN(prayer_acid_fog, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("The caustic acid fog is already hanging "
			 "over this place.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("Try to create acid fog later.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= UMIN(1, level / 20);
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	paf = aff_new(TO_AFFECTS, sn);
	paf->type      = sn;
	paf->duration  = number_range(5, 7);
	paf->level     = level;
	paf->modifier  = 0;
	paf->bitvector = 0;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Caustic acid drops fill air.", ch, NULL, NULL, TO_ALL);
}

/* Domain: water
 * Simple damage spell.
 */
SPELL_FUN(prayer_ice_storm, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam = calc_spell_damage(ch, level, sn);

	if (saves_spell(level, victim, DAM_COLD))
		dam /= 2;

	act("Throng ice flakes surrounds $N!", ch, NULL, victim, TO_CHAR);
	act("Throng ice flakes surrounds you. Very cold!",
	    ch, NULL, victim, TO_VICT);
	act("Throng ice flakes surrounds $N!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}
/* Domain: water
 * Make room dark.
 */
SPELL_FUN(prayer_fog_cloud, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("Fog clouds fill all around you already.", ch);
		return;
	}

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 3;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act("Fog clouds surround you.", ch, NULL, NULL, TO_ALL);
}
/* Domain: air
 * Prayer remove fog clouds.
 */
SPELL_FUN(prayer_blast_of_wind, sn, level, ch, vo)
{
	if (is_sn_affected_room(ch->in_room, "fog cloud")) {
		affect_strip_room(ch->in_room, "fog cloud");
		act("The blast of wind removes all cloud!",
		    ch, NULL, NULL, TO_ALL);
	}
}
/* Domain: air
 * grants resistance against DAM_AIR, DAM_COLD
 */
SPELL_FUN(prayer_air_ward, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_wards(ch, ch))
		return;

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_AIR;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_COLD;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection against air and cold.",
	    ch, NULL, NULL, TO_CHAR);
}
/* Domain: water
 * grants resistance against DAM_WATER, DAM_COLD, DAM_ACID
 */
SPELL_FUN(prayer_water_ward, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_wards(ch, ch))
		return;

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_WATER;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_COLD;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_ACID;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection against cold, "
	    "acid and water.", ch, NULL, NULL, TO_CHAR);
}
/* Domain: earth
 * grants resistance against DAM_EARTH
 */
SPELL_FUN(prayer_earth_ward, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_wards(ch, ch))
		return;

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_EARTH;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection against earth.",
	    ch, NULL, NULL, TO_CHAR);
}
/* Domain: fire
 * grants resistance against DAM_FIRE
 */
SPELL_FUN(prayer_fire_ward, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_wards(ch, ch))
		return;

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_FIRE;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection against fire.",
	    ch, NULL, NULL, TO_CHAR);
}
/* Domain: death
 * Grants protection from damage, strip when paf->modifier became >= 0
 * also a part of damage returns to opponent
 */
SPELL_FUN(prayer_death_ward, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_sanctuary(ch, ch))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->duration	= -1;
	paf->level	= level;
	INT(paf->location)= APPLY_VDAMAGE;
	paf->modifier	= - level * 10 + 100;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection!", ch, NULL, NULL, TO_CHAR);
}
/* Domain: death
 * inflict damage to target
 * decrease target's stats
 */
SPELL_FUN(prayer_spear_of_death, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (ch == victim) {
		act_char("Even you are not so silly.", ch);
		return;
	}

        act("You create a spear from darkness and throw it toward $N!",
	    ch, NULL, victim, TO_CHAR);
	act("$n creates a spear from darkness and throws it toward $N!",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n creates a spear from darkness and throws it toward you!",
	    ch, NULL, victim, TO_VICT);

	if (!is_sn_affected(victim, sn)
	&&  !IS_SET(victim->form, FORM_UNDEAD)
	&&  !saves_spell(level, victim, DAM_NEGATIVE)) {
		AFFECT_DATA *paf;
		int mod = get_curr_stat(ch, STAT_INT);

		paf = aff_new(TO_AFFECTS, sn);
		paf->duration	= mod / 4;
		paf->level	= level;
		INT(paf->location)= APPLY_STR;
		paf->modifier	= - mod / 5;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_INT;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_WIS;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DEX;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_CON;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_LUCK;
		paf->modifier	= - mod;
		affect_to_char(victim, paf);
		aff_free(paf);

		act_char("A spear of darkness sears your body!", victim);
		act("A spear of darkness sears $n's body!",
		    victim, NULL, NULL, TO_ROOM);
	}

	dam = calc_spell_damage(ch, level, sn);
	if (IS_SET(victim->form, FORM_UNDEAD)
	||  saves_spell(level, victim, DAM_NEGATIVE))
		dam /= 3;

	damage(ch, victim, dam, sn, DAM_F_SHOW);
}
/* Domain: death
 * AFF_SLEEP to target
 * undead has immune
 */
SPELL_FUN(prayer_sleep_of_grave, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (ch == victim)
		act_char("Even you is not so stupid.", ch);

	if (IS_AFFECTED(victim, AFF_SLEEP)
	||  IS_SET(victim->form, FORM_UNDEAD)
	||  IS_SET(victim->form, FORM_CONSTRUCT)
	||  saves_spell(level, victim, DAM_CHARM))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 1 + level/10;
	paf->bitvector	= AFF_SLEEP;
	affect_to_char(victim, paf);
	aff_free(paf);

	if (IS_AWAKE(victim)) {
		act_char("Irresistible sleep holds your body.", victim);
		act("$n puts $N into nightmares.", ch, NULL, victim,
		    TO_NOTVICT);
		act("You put $N into nightmares.", ch, NULL, victim, TO_CHAR);
		victim->position = POS_SLEEPING;
	}
}
/* Domain: air
 * open closed, locked doors
 */
SPELL_FUN(prayer_wind_blow, sn, level, ch, vo)
{
	char arg[MAX_INPUT_LENGTH];
	int chance = 0;
	int door;
	EXIT_DATA *pexit;

	target_name = one_argument(target_name, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Which direction?", ch);
		return;
	}

	if ((door = find_door(ch, arg)) < 0)
		return;

	pexit = ch->in_room->exit[door];

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		act("A mystical shield protects $d.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return;
	}

	chance = level / 5 + get_curr_stat(ch, STAT_INT) + get_skill(ch,sn) / 5;

	act("You create wind fist and send it toward $d.",
	    ch, &pexit->short_descr, NULL, TO_CHAR);
	act("$n create wind fist and send it toward $d.",
	    ch, &pexit->short_descr, NULL, TO_ROOM);

	if (number_percent() < chance) {
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit_rev;

		REMOVE_BIT(pexit->exit_info, EX_LOCKED | EX_CLOSED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act("Wind opens $d!", ch, &pexit->short_descr, NULL, TO_ALL);

		/* open the other side */
		if ((to_room = pexit->to_room.r) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  pexit_rev->to_room.r == ch->in_room) {
			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED | EX_LOCKED);
			act("$d opens.", to_room->people,
			    &pexit_rev->short_descr, NULL, TO_ROOM);
		}
	} else {
		act("Wind is not strong enought to open $d.",
		    ch, &pexit->short_descr, NULL, TO_ALL);
	}
}
/* Domain: air
 * aka teleport, can't be used in indoors room, can't be used on self
 */
SPELL_FUN(prayer_tornado, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *target_room;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You can't use this ability so soon again.", ch);
		return;
	}

	if (victim == ch) {
		act_char("It's too dangerous.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_INDOORS)) {
		act_char("You can't create tornado here.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->duration	= level / 15;
	paf->level	= level;
	affect_to_char(ch, paf);
	aff_free(paf);

	if (IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  (!IS_NPC(ch) && victim->fighting != NULL)
	||  (!IS_NPC(ch) && !in_PK(ch, victim))
	||  (victim != ch &&
	     saves_spell(level - 5, victim, DAM_OTHER))) {
		act("Tornado picks up $N and drops $gN{him} right away.",
		    ch, NULL, victim, TO_CHAR);
		act("Tornado picks up $N and drops $gN{him} right away.",
		    ch, NULL, victim, TO_NOTVICT);
		act("Tornado picks up you and drops right away.",
		    ch, NULL, victim, TO_VICT);
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
			act("Tornado picks up $N and drops $gN{him} right "
			    "away.", ch, NULL, victim, TO_CHAR);
			act("Tornado picks up $N and drops $gN{him} right "
			    "away.", ch, NULL, victim, TO_NOTVICT);
			act("Tornado picks up you and drops right away.",
			    ch, NULL, victim, TO_VICT);
                        return;
                }
        }

	for (;;) {
		target_room = get_random_room(victim, NULL);
		if (!IS_SET(target_room->room_flags, ROOM_INDOORS))
			break;
	}

	teleport_char(victim, ch, target_room,
		      "Tornado picks up $n and $gn{he} vanishes!",
		      "Tornado picks up you!",
		      "Tornado appears from nowhere and drops $n to the ground.");

}
SPELL_FUN(prayer_ice_sphere, sn, level, ch, vo)
{
        AFFECT_DATA *paf;

        if (is_sn_affected(ch, sn)) {
                act_puts("Ice sphere already surrounds you.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
        };

	if (!can_cast_sanctuary(ch, ch))
		return;

	paf = aff_new(TO_AFFECTS, sn);
	paf->type      = sn;
	paf->level     = level;
	paf->duration  = UMAX (2, level/4);
	paf->modifier  = 0;
	paf->bitvector = 0;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Water freezes around you forming sparkling ice sphere.",
	    ch, NULL, NULL, TO_CHAR);
	act("Water freezes around $n forming sparkling ice sphere.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_cloak_of_death, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (!can_cast_wards(ch, ch))
		return;

	paf = aff_new(TO_RESISTS, sn);
	paf->duration	= level / 10;
	paf->level	= level;
	INT(paf->location)= DAM_NEGATIVE;
	paf->modifier	= 95;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("Your god grant you protection against negative.",
	    ch, NULL, NULL, TO_CHAR);
}

SPELL_FUN(prayer_water_walk, sn, level, ch, vo)
{
	if (!is_sn_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->duration	= level;
		paf->level	= level;
		affect_to_char(ch, paf);
		aff_free(paf);

		act("Now water will be solid for you.",
		    ch, NULL, NULL, TO_CHAR);
	} else
		act_char("You are already have water walking ability.", ch);
}

SPELL_FUN(prayer_breath_under_water, sn, level, ch, vo)
{
	if (!is_sn_affected(ch, sn)) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, sn);
		paf->duration	= level;
		paf->level	= level;
		affect_to_char(ch, paf);
		aff_free(paf);

		act("Your God grants you ability to breath under water.",
		    ch, NULL, NULL, TO_CHAR);
	} else
		act_char("You are already can breath under water.", ch);
}

SPELL_FUN(prayer_geyser, sn, level, ch, vo)
{
	CHAR_DATA *victim;
	int dam;

	if (ch->in_room->sector_type == SECT_UNDERWATER
	||  IS_WATER(ch->in_room)) {
		act_char("You cannot create a geyser in deep sea.", ch);
		return;
	}

	act("$n rise $gn{his} hands and call divine help to create a"
	    " geyser.", ch, NULL, NULL, TO_ROOM);
	act_char("You call for divine help to create a geyser.", ch);

	dam = calc_spell_damage(ch, level, sn);

	foreach (victim, char_in_room(ch->in_room)) {
		if (is_safe_spell(ch, victim, TRUE))
			continue;

		if (saves_spell(level, victim, DAM_WATER))
			dam /= 3;

		if (ch->in_room->sector_type == SECT_MOUNTAIN)
			dam *= 4 / 3;

		if (ch->in_room->sector_type == SECT_HILLS)
			dam *= 6 / 5;

		damage(ch, victim, dam, sn, DAM_F_SHOW);
	} end_foreach(victim);

	act("$n's geyser dries up and dissapears.", ch, NULL, NULL, TO_ROOM);
	act_char("Your geyser dries up and dissapears.", ch);
}

static int water_elemental_stats[MAX_STAT] = {
	23, 23, 20, 24, 16, 25
};

SPELL_FUN(prayer_water_elemental, sn, level, ch, vo)
{
	CHAR_DATA *elemental;
	AFFECT_DATA *paf;
	int i;
	int modifier = 0;

	if (is_sn_affected(ch, sn)) {
		act_puts("You cannot summon water elemental right now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!can_summon_creature(ch))
		return;

	if (ch->in_room->sector_type == SECT_DESERT) {
		act_char("There is not enought water.", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_UNDERWATER
	||  IS_WATER(ch->in_room))
		modifier = 3;


	act("You notice some strange motions around you.",
	    ch, NULL, NULL, TO_ALL);

	elemental = create_mob(MOB_VNUM_WATER_ELEMENTAL, 0);
	if (elemental == NULL) {
		act("But nothing else happens.", ch, NULL, NULL, TO_ALL);
		return;
	}

	for (i = 0; i < MAX_STAT; i++) {
		elemental->perm_stat[i] = water_elemental_stats[i];
	}

	SET_HIT(elemental, ch->perm_hit);
	SET_MANA(elemental, ch->max_mana);
	elemental->level = level + modifier;
	for (i = 0; i < 4; i++)
		elemental->armor[i] = interpolate(elemental->level, 100, -100);
	elemental->gold = elemental->silver = 0;
	NPC(elemental)->dam.dice_number =
	    number_range(level/15, level/10) + modifier;
	NPC(elemental)->dam.dice_type   = number_range(level/3, level/2);
	elemental->damroll  = 0;

	if (ch->in_room->sector_type == SECT_UNDERWATER
	||  IS_WATER(ch->in_room)) {
		act("Water becomes solid and $N appears.",
		    ch, NULL, elemental, TO_ALL);
	} else
		act("$N appears from nowhere.", ch, NULL, elemental, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 50 - modifier;
	affect_to_char(ch, paf);
	aff_free(paf);

	elemental->master = elemental->leader = ch;

	char_to_room(elemental, ch->in_room);
}

static struct mutually_exclusive_spells mx_wards[] = {
	{ "air ward", "You are already protected by air ward.",
		"$N is already protected by air ward."},
	{ "water ward", "You are already protected by water ward.",
		"$N is already protected by water ward."},
	{ "fire ward", "You are already protected by fire ward.",
		"$N is already protected by fire ward."},
	{ "earth ward", "You are already protected by earth ward",
		"$N is already protected by earth ward."},
	{ "cloak of death", "Black cloak already surrounds you.",
		"Black cloak already surrounds $N."},
	{ NULL, NULL, NULL }
};

static bool
can_cast_wards(CHAR_DATA *ch, CHAR_DATA *victim)
{
	struct mutually_exclusive_spells *p = mx_wards;

	for (; p->sn; p++) {
		if (is_sn_affected(victim, p->sn)) {
			if (ch == victim)
				act_char(p->self_cast_msg, ch);
			else {
				act(p->other_cast_msg, ch, NULL, victim,
				    TO_CHAR);
			}
			return FALSE;
		}
	}

	return TRUE;
}

static int fire_elemental_stats[MAX_STAT] = {
	23, 23, 20, 24, 16, 25
};

SPELL_FUN(prayer_fire_elemental, sn, level, ch, vo)
{
	CHAR_DATA *elemental;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_puts("You cannot summon fire elemental right now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!can_summon_creature(ch))
		return;

	if (ch->in_room->sector_type == SECT_UNDERWATER
	||  IS_WATER(ch->in_room)) {
		act_char("You can't summon fire elemental here.", ch);
		return;
	}

	act("You notice some strange motions around you.",
	    ch, NULL, NULL, TO_ALL);

	elemental = create_mob(MOB_VNUM_FIRE_ELEMENTAL, 0);
	if (elemental == NULL) {
		act("But nothing else happens.", ch, NULL, NULL, TO_ALL);
		return;
	}

	for (i = 0; i < MAX_STAT; i++) {
		elemental->perm_stat[i] = fire_elemental_stats[i];
	}

	SET_HIT(elemental, ch->perm_hit);
	SET_MANA(elemental, ch->max_mana);
	elemental->level = level;
	for (i = 0; i < 4; i++)
		elemental->armor[i] = interpolate(elemental->level, 100, -100);
	elemental->gold = elemental->silver = 0;
	NPC(elemental)->dam.dice_number =
	    number_range(level/15, level/10);
	NPC(elemental)->dam.dice_type   = number_range(level/3, level/2);
	elemental->damroll  = 0;

	act("$N appears from nowhere.", ch, NULL, elemental, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 50;
	affect_to_char(ch, paf);
	aff_free(paf);

	elemental->master = elemental->leader = ch;

	char_to_room(elemental, ch->in_room);
}

static int earth_elemental_stats[MAX_STAT] = {
	23, 23, 20, 24, 16, 25
};

SPELL_FUN(prayer_earth_elemental, sn, level, ch, vo)
{
	CHAR_DATA *elemental;
	AFFECT_DATA *paf;
	int i;

	if (is_sn_affected(ch, sn)) {
		act_puts("You cannot summon earth elemental right now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!can_summon_creature(ch))
		return;

	if (ch->in_room->sector_type == SECT_UNDERWATER
	||  IS_WATER(ch->in_room)) {
		act_char("You can't summon earth elemental here.", ch);
		return;
	}

	act("You notice some strange motions around you.",
	    ch, NULL, NULL, TO_ALL);

	elemental = create_mob(MOB_VNUM_EARTH_ELEMENTAL, 0);
	if (elemental == NULL) {
		act("But nothing else happens.", ch, NULL, NULL, TO_ALL);
		return;
	}

	for (i = 0; i < MAX_STAT; i++) {
		elemental->perm_stat[i] = earth_elemental_stats[i];
	}

	SET_HIT(elemental, ch->perm_hit);
	SET_MANA(elemental, ch->max_mana);
	elemental->level = level;
	for (i = 0; i < 4; i++)
		elemental->armor[i] = interpolate(elemental->level, 100, -100);
	elemental->gold = elemental->silver = 0;
	NPC(elemental)->dam.dice_number =
	    number_range(level/15, level/10);
	NPC(elemental)->dam.dice_type   = number_range(level/3, level/2);
	elemental->damroll  = 0;

	act("$N appears from nowhere.", ch, NULL, elemental, TO_ALL);

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= 50;
	affect_to_char(ch, paf);
	aff_free(paf);

	elemental->master = elemental->leader = ch;

	char_to_room(elemental, ch->in_room);
}

SPELL_FUN(prayer_produce_flame, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch;
	AFFECT_DATA *paf;

	act("You produce fire cloud!", ch, NULL, NULL, TO_CHAR);
	act("$n produce fire cloud!", ch, NULL, NULL, TO_ROOM);

	if (!saves_spell(level, victim, DAM_FIRE)) {
		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= UMIN(1, level / 10);
		paf->bitvector	= AFF_BLIND;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("$N is blinded by flame!", ch, NULL, victim, TO_CHAR);
		act("$N is blinded by flame!", ch, NULL, victim, TO_NOTVICT);
		act("You are blinded by flame!", ch, NULL, victim, TO_VICT);
	}

	foreach (vch, char_in_room(ch->in_room)) {
		if (is_safe_spell(ch, vch, TRUE)
		||  vch == victim)
			continue;

		if (saves_spell(level, vch, DAM_FIRE)
		||  number_bits(1) == 0)
			return;

		paf = aff_new(TO_AFFECTS, sn);
		paf->level	= level;
		paf->duration	= UMIN(1, level / 20);
		paf->bitvector	= AFF_BLIND;
		affect_to_char(vch, paf);
		aff_free(paf);

		act("$N is blinded by flame!", ch, NULL, vch, TO_CHAR);
		act("$N is blinded by flame!", ch, NULL, vch, TO_NOTVICT);
		act("You are blinded by flame!", ch, NULL, vch, TO_VICT);
	} end_foreach(vch);
}

/* Domain: death
 * block victim's regeneration ability
 */
SPELL_FUN(prayer_death_breathing, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (victim == ch) {
		act_char("You are not so silly!", ch);
		return;
	}

	if (is_sn_affected(victim, sn)) {
		act("$N is already on the way to death.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	act("You tried to cloak $N with death breathing...",
	    ch, NULL, victim, TO_CHAR);
	act("$n tried to cloak you with death breathing...",
	    ch, NULL, victim, TO_VICT);

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act("$N avoid your influence.", ch, NULL, victim, TO_CHAR);
		act("You avoid $n's influence.", ch, NULL, victim, TO_VICT);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level = level;
	paf->duration = level / 5;
	affect_to_char(victim, paf);
	aff_free(paf);

	act("Death is now behind $N!", ch, NULL, victim, TO_CHAR);
	act("Death is now behind you!", ch, NULL, victim, TO_VICT);
}

SPELL_FUN(prayer_rock_skin, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("Your skin are already protected by rocks.", ch);
		return;
	}

	paf = aff_new(TO_RESISTS, sn);
	paf->level	= level;
	paf->duration	= level / 10;
	paf->modifier	= 33;
	INT(paf->location)= DAM_SLASH;
	affect_to_char(ch, paf);

	INT(paf->location)= DAM_PIERCE;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Rocks stood out on yours skin protecting you.", ch);
}

SPELL_FUN(prayer_spike_stones, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	act_puts("You send a torrent of spike stones at $N.",
		 ch, NULL, victim, TO_CHAR, POS_DEAD);
	act("$n sends a torrent of spike stones at you!",
	    ch, NULL, victim, TO_VICT);
	inflict_spell_damage(ch, victim, level, sn);
	spellfun("slow", NULL, 2 * level / 3, ch, vo);
}

SPELL_FUN(prayer_strength_of_earth, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, sn)) {
		act_char("You are already full of the strength of earth.",
		         ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 3;
	paf->modifier	= level / 9;

	INT(paf->location) = APPLY_HITROLL;
	affect_to_char(ch, paf);

	INT(paf->location) = APPLY_DAMROLL;
	affect_to_char(ch, paf);

	paf->modifier = -level;
	INT(paf->location) = APPLY_AC;
	affect_to_char(ch, paf);

	paf->modifier = level / 15;
	INT(paf->location) = APPLY_STR;
	affect_to_char(ch, paf);

	paf->modifier = level / 15;
	INT(paf->location) = APPLY_CON;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("The strength of the earth fills you.", ch);
	act("$n looks a bit stocky now.", ch, NULL, NULL, TO_ROOM);
}

/* domain: earth
 * Prayer filles soil of a room with sharp barbs.
 * Barbs damages all creatures in room (owner has immunity).
 * Enter room event: event_enter_barbs
 * Update room event: event_update_barbs
 */
SPELL_FUN(prayer_soil_barbs, sn, level, ch, vo)
{
	AFFECT_DATA *paf;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("This room is protected by gods.", ch);
		return;
	}

	if (is_sn_affected_room(ch->in_room, sn)) {
		act_char("This room has already been full of barbs.", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_AIR
	||  ch->in_room->sector_type == SECT_WATER_SWIM
	||  ch->in_room->sector_type == SECT_WATER_NOSWIM
	||  ch->in_room->sector_type == SECT_UNDERWATER) {
		act_char("You can't reach the ground.", ch);
		return;
	}

	if (is_sn_affected(ch, sn)) {
		act_char("Try to grow barbs later.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	paf->type      = sn;
	paf->duration  = number_range(5, 7);
	paf->level     = level;
	paf->modifier  = 0;
	paf->bitvector = 0;
	affect_to_char(ch, paf);
	aff_free(paf);

	paf = aff_new(TO_ROOM_AFFECTS, sn);
	paf->level	= level;
	paf->duration	= level / 15;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);

	act_char("Many sharp barbs step out from the soil.", ch);
	act("Many sharp barbs step out from the soil.",
	    ch, NULL, NULL, TO_ROOM);
}

SPELL_FUN(prayer_meteor_shower, sn, level, ch, vo)
{
	CHAR_DATA *vch;

	act_char("Meteors rain down from the sky!", ch);
	act("$n makes meteors rain down from the sky.", ch, NULL, NULL, TO_ROOM);

	foreach (vch, char_in_room(ch->in_room)) {
		if (!is_safe_spell(ch, vch, TRUE))
			inflict_spell_damage(ch, vch, level, sn);
	} end_foreach(vch);
}

SPELL_FUN(prayer_hail_of_stones, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *mount;
	int dam;

	act_puts("You hit $N by a hail of stones.",
	         ch, NULL, victim, TO_CHAR, POS_DEAD);
	act("$n hits you by a hail stones!", ch, NULL, victim, TO_VICT);

	dam = calc_spell_damage(ch, level, sn);

	if (!saves_spell(level, victim, DAM_EARTH)) {
		if ((mount = MOUNTED(victim))) {
			act_puts("You fall down from $N.",
				 victim, NULL, mount, TO_CHAR, POS_DEAD);
			act("$n falls down from $N.",
			    victim, NULL, mount, TO_NOTVICT);
			act_puts("$n falls down from you.",
				 victim, NULL, mount, TO_VICT, POS_SLEEPING);

			victim->riding = FALSE;
			mount->riding = FALSE;
		}

		if (IS_AFFECTED(victim, AFF_FLYING)) {
			REMOVE_BIT(victim->affected_by, AFF_FLYING);
			act_char("You can't fly under such a hail and touch the ground.",
				 victim);
			act("$n stops flying.", victim, NULL, NULL, TO_NOTVICT);
		}
	} else
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

/* Domain: fire
 * room attack
 */
SPELL_FUN(prayer_incendiary_cloud, sn, level, ch, vo)
{
	CHAR_DATA *vch;

	act_char("You create incendiary cloud!", ch);
	act("$n creates incendiary cloud!", ch, NULL, NULL, TO_ROOM);

	foreach (vch, char_in_room(ch->in_room)) {
		int dam;

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		dam = calc_spell_damage(ch, level, sn);
		if (saves_spell(level, vch, DAM_FIRE))
			dam /= 2;

		damage(ch, vch, dam, sn, DAM_F_SHOW);
	} end_foreach(vch);
}

SPELL_FUN(prayer_unholy_aura, sn, level, ch, vo)
{
	CHAR_DATA *vch = vo;
	AFFECT_DATA *paf;

	if (is_sn_affected(vch, sn)) {
		act_char("You are already protected by a unholy aura.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, sn);
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= UMAX(1, level / 8);
	paf->bitvector	= 0;
	paf->duration	= level / 4;
	affect_to_char(vch, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	=  -UMAX(1, level / 8);
	affect_to_char(vch, paf);
	aff_free(paf);

	act_char("You feel a unholy aura around you.", vch);
}

SPELL_FUN(prayer_unholy_blight, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (IS_GOOD(ch)) {
		victim = ch;
		act_char("Gods are indignant with your action!", ch);
	}

	if (victim != ch) {
		act("$n raises $s hand, and a wave of unholy blight "
		    "spreads forth!", ch, NULL, NULL, TO_ROOM);
		act_char("You raise your hand and a wave of unholy "
			 "blight spreads forth!", ch);
	}

	if (IS_EVIL(victim)) {
		act("$n seems unharmed by the blight.",
		    victim, NULL, victim, TO_ROOM);
		act_char("The blight seems powerless to affect you.", victim);
		return;
	}

	dam = calc_spell_damage(ch, level, sn);
	if (saves_spell(level, victim, DAM_EVIL))
		dam /= 2;

	spellfun("poison", NULL, 4 * level / 3, ch, victim);
	damage(ch, victim, dam, sn, DAM_F_SHOW);
}

SPELL_FUN(prayer_inspire_weapon, sn, level, ch, vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if (obj->pObjIndex->item_type != ITEM_WEAPON) {
		act_char("That isn't a weapon.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be inspired.", ch);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_VORPAL)) {
		act("$p is already inspired with a soul.",
		    ch, obj, NULL, TO_CHAR);
		return;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)) {
		act("You can't seem to inspire $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	paf = aff_new(TO_WEAPON, sn);
	paf->level	= level / 2;
	paf->duration	= level / 8;
	paf->bitvector	= WEAPON_VORPAL;
	affect_to_obj(obj, paf);
	aff_free(paf);

	act("$p is inspired with a part of your soul.", ch, obj, NULL, TO_CHAR);
	act("Now it can slay enemies without your help.",
	    ch, obj, NULL, TO_CHAR);
}

SPELL_FUN(prayer_blasphemy, sn, level, ch, vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf;

	if (victim == ch)
		act_char("That wouldn't work.", ch);

	if (IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("You cann't use this power here.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_char("You is not powerful enough.", ch);
		return;
	}

	act("$n utters blasphemy against $N!.",
	    ch, NULL, victim, TO_NOTVICT);
	act("You utter blasphemy against $N!",
	    ch, NULL, victim, TO_CHAR);

	if (!can_hear(victim)) {
		act("Your victim ignores all you attempts - $gN{he}'s deaf!",
		    ch, NULL, victim, TO_CHAR);
		act("You see $n's lips moving, but you cann't hear anything!",
		    ch, NULL, victim, TO_VICT);
		return;
	}

	act("$n utters blasphemy against you!",
	    ch, NULL, victim, TO_VICT);

	if (is_sn_affected(victim, sn)) {
		act("$N is already affected by blasphemy, "
		    "you cann't do more damage.", ch, NULL, victim, TO_CHAR);
		act_char("You feel unclean but it passes quickly.", victim);
		return;
	}

	if (IS_NPC(victim)
	&&  IS_CLAN_GUARD(victim)) {
		act("$N shivers momentarily but it passes.",
		    ch, NULL, NULL, TO_NOTVICT);
		return;
	}

	if (saves_spell(level, victim, DAM_EVIL)) {
		act("$n shivers momentarily but it passes.",
		    victim, NULL, NULL, TO_ROOM);
		act("You feel a brief pain, but it passes.",
		    victim, NULL, NULL, TO_CHAR);
		return;
	}

	if (level >= 4 * LEVEL_HERO / 5
	&&  !saves_spell(level - 5, victim, DAM_EVIL)) {
		act("$n's face distorten and $gn{his} head explodes!",
		    victim, NULL, NULL, TO_ROOM);
		act("$N's blasphemy overwhelms your head "
		    "causing it to explode!", victim, NULL, ch, TO_CHAR);
		raw_kill(ch, victim);
		return;
	}

	act("You feel very unclean.",
	    victim, NULL, NULL, TO_CHAR);

	paf = aff_new(TO_AFFECTS, sn);
	paf->bitvector	= AFF_WEAKEN;

	if (level >= LEVEL_HERO / 3)
		paf->bitvector |= AFF_PLAGUE;

	if (level >= 2 * LEVEL_HERO / 3
	&& number_percent() < 70) {
		act("You feel so bad, you cann't do anything!.",
		    victim, NULL, NULL, TO_CHAR);
		paf->bitvector |= AFF_STUN;
	}

	paf->level	= level;
	paf->duration	= 1 + number_range(level / 20, level / 10);
	INT(paf->location)= APPLY_STR;
	paf->modifier	= -(2 + level / 7);
	affect_to_char(victim, paf);
	aff_free(paf);
}
