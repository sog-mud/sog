/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: spellfn.h,v 1.11 1999-02-23 07:55:42 kostik Exp $
 */

#ifndef _SPELLFN_H_
#define _SPELLFN_H_

extern NAMEDP spellfn_table[];

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(spell_acid_blast);
DECLARE_SPELL_FUN(spell_armor);
DECLARE_SPELL_FUN(spell_bless);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_burning_hands);
DECLARE_SPELL_FUN(spell_call_lightning);
DECLARE_SPELL_FUN(spell_calm);
DECLARE_SPELL_FUN(spell_cancellation);
DECLARE_SPELL_FUN(spell_cause_critical);
DECLARE_SPELL_FUN(spell_cause_light);
DECLARE_SPELL_FUN(spell_cause_serious);
DECLARE_SPELL_FUN(spell_healing_light);
DECLARE_SPELL_FUN(spell_chain_lightning);
DECLARE_SPELL_FUN(spell_charm_person);
DECLARE_SPELL_FUN(spell_chill_touch);
DECLARE_SPELL_FUN(spell_colour_spray);
DECLARE_SPELL_FUN(spell_continual_light);
DECLARE_SPELL_FUN(spell_control_weather);
DECLARE_SPELL_FUN(spell_create_food);
DECLARE_SPELL_FUN(spell_create_rose);
DECLARE_SPELL_FUN(spell_create_spring);
DECLARE_SPELL_FUN(spell_create_water);
DECLARE_SPELL_FUN(spell_cure_blindness);
DECLARE_SPELL_FUN(spell_cure_critical);
DECLARE_SPELL_FUN(spell_cure_disease);
DECLARE_SPELL_FUN(spell_cure_light);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_cure_serious);
DECLARE_SPELL_FUN(spell_curse);
DECLARE_SPELL_FUN(spell_anathema);
DECLARE_SPELL_FUN(spell_demonfire);
DECLARE_SPELL_FUN(spell_detect_evil);
DECLARE_SPELL_FUN(spell_detect_good);
DECLARE_SPELL_FUN(spell_detect_hidden);
DECLARE_SPELL_FUN(spell_detect_invis);
DECLARE_SPELL_FUN(spell_detect_magic);
DECLARE_SPELL_FUN(spell_detect_poison);
DECLARE_SPELL_FUN(spell_detect_undead);
DECLARE_SPELL_FUN(spell_disintegrate);
DECLARE_SPELL_FUN(spell_dispel_evil);
DECLARE_SPELL_FUN(spell_dispel_good);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_earthquake);
DECLARE_SPELL_FUN(spell_enchant_armor);
DECLARE_SPELL_FUN(spell_enchant_weapon);
DECLARE_SPELL_FUN(spell_hunger_weapon);
DECLARE_SPELL_FUN(spell_energy_drain);
DECLARE_SPELL_FUN(spell_faerie_fire);
DECLARE_SPELL_FUN(spell_faerie_fog);
DECLARE_SPELL_FUN(spell_farsight);
DECLARE_SPELL_FUN(spell_fireball);
DECLARE_SPELL_FUN(spell_fireproof);
DECLARE_SPELL_FUN(spell_flamestrike);
DECLARE_SPELL_FUN(spell_floating_disc);
DECLARE_SPELL_FUN(spell_fly);
DECLARE_SPELL_FUN(spell_frenzy);
DECLARE_SPELL_FUN(spell_gate);
DECLARE_SPELL_FUN(spell_giant_strength);
DECLARE_SPELL_FUN(spell_harm);
DECLARE_SPELL_FUN(spell_haste);
DECLARE_SPELL_FUN(spell_heal);
DECLARE_SPELL_FUN(spell_heat_metal);
DECLARE_SPELL_FUN(spell_holy_word);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_infravision);
DECLARE_SPELL_FUN(spell_invisibility);
DECLARE_SPELL_FUN(spell_know_alignment);
DECLARE_SPELL_FUN(spell_lightning_bolt);
DECLARE_SPELL_FUN(spell_locate_object);
DECLARE_SPELL_FUN(spell_magic_missile);
DECLARE_SPELL_FUN(spell_mass_healing);
DECLARE_SPELL_FUN(spell_mass_invis);
DECLARE_SPELL_FUN(spell_nexus);
DECLARE_SPELL_FUN(spell_pass_door);
DECLARE_SPELL_FUN(spell_plague);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_portal);
DECLARE_SPELL_FUN(spell_protection_evil);
DECLARE_SPELL_FUN(spell_protection_good);
DECLARE_SPELL_FUN(spell_ray_of_truth);
DECLARE_SPELL_FUN(spell_recharge);
DECLARE_SPELL_FUN(spell_refresh);
DECLARE_SPELL_FUN(spell_remove_curse);
DECLARE_SPELL_FUN(spell_sanctuary);
DECLARE_SPELL_FUN(spell_black_shroud);
DECLARE_SPELL_FUN(spell_shocking_grasp);
DECLARE_SPELL_FUN(spell_shield);
DECLARE_SPELL_FUN(spell_sleep);
DECLARE_SPELL_FUN(spell_slow);
DECLARE_SPELL_FUN(spell_stone_skin);
DECLARE_SPELL_FUN(spell_summon);
DECLARE_SPELL_FUN(spell_teleport);
DECLARE_SPELL_FUN(spell_ventriloquate);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_word_of_recall);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_general_purpose);
DECLARE_SPELL_FUN(spell_high_explosive);
DECLARE_SPELL_FUN(spell_mana_transfer);
DECLARE_SPELL_FUN(spell_transform);
DECLARE_SPELL_FUN(spell_demon_summon);
DECLARE_SPELL_FUN(spell_scourge);
DECLARE_SPELL_FUN(spell_iceball);
DECLARE_SPELL_FUN(spell_manacles);
DECLARE_SPELL_FUN(spell_mental_knife);
DECLARE_SPELL_FUN(spell_guard_call);
DECLARE_SPELL_FUN(spell_nightwalker);
DECLARE_SPELL_FUN(spell_eyes_of_intrigue);
DECLARE_SPELL_FUN(spell_shadow_cloak);
DECLARE_SPELL_FUN(spell_nightfall);
DECLARE_SPELL_FUN(spell_confuse);
DECLARE_SPELL_FUN(spell_garble);
DECLARE_SPELL_FUN(spell_terangreal);
DECLARE_SPELL_FUN(spell_kassandra);
DECLARE_SPELL_FUN(spell_matandra);
DECLARE_SPELL_FUN(spell_sebat);
DECLARE_SPELL_FUN(spell_amnesia);
DECLARE_SPELL_FUN(spell_chaos_blade);
DECLARE_SPELL_FUN(spell_tattoo);
DECLARE_SPELL_FUN(spell_remove_tattoo);
DECLARE_SPELL_FUN(spell_wrath);
DECLARE_SPELL_FUN(spell_randomizer);
DECLARE_SPELL_FUN(spell_tesseract);
DECLARE_SPELL_FUN(spell_stalker);
DECLARE_SPELL_FUN(spell_brew);
DECLARE_SPELL_FUN(spell_shadowlife);
DECLARE_SPELL_FUN(spell_ruler_badge);
DECLARE_SPELL_FUN(spell_remove_badge);
DECLARE_SPELL_FUN(spell_dragon_strength);
DECLARE_SPELL_FUN(spell_dragon_breath);
DECLARE_SPELL_FUN(spell_golden_aura);
DECLARE_SPELL_FUN(spell_dragonplate);
DECLARE_SPELL_FUN(spell_squire);
DECLARE_SPELL_FUN(spell_dragonsword);
DECLARE_SPELL_FUN(spell_entangle);
DECLARE_SPELL_FUN(spell_love_potion);
DECLARE_SPELL_FUN(spell_deafen);
DECLARE_SPELL_FUN(spell_protective_shield);
DECLARE_SPELL_FUN(spell_disperse);
DECLARE_SPELL_FUN(spell_bark_skin);
DECLARE_SPELL_FUN(spell_acute_vision);
DECLARE_SPELL_FUN(spell_ranger_staff);
DECLARE_SPELL_FUN(spell_hellfire);
DECLARE_SPELL_FUN(spell_dragons_breath);
DECLARE_SPELL_FUN(spell_sand_storm);
DECLARE_SPELL_FUN(spell_scream);
DECLARE_SPELL_FUN(spell_attract_other);
DECLARE_SPELL_FUN(spell_animate_dead);
DECLARE_SPELL_FUN(spell_enhanced_armor);
DECLARE_SPELL_FUN(spell_meld_into_stone);
DECLARE_SPELL_FUN(spell_find_object);
DECLARE_SPELL_FUN(spell_shielding);
DECLARE_SPELL_FUN(spell_web);
DECLARE_SPELL_FUN(spell_group_defense);
DECLARE_SPELL_FUN(spell_inspire);
DECLARE_SPELL_FUN(spell_link);
DECLARE_SPELL_FUN(spell_mass_sanctuary);
DECLARE_SPELL_FUN(spell_mend);
DECLARE_SPELL_FUN(spell_wolf);
DECLARE_SPELL_FUN(spell_lion_help);
DECLARE_SPELL_FUN(spell_magic_jar);
DECLARE_SPELL_FUN(spell_turn);
DECLARE_SPELL_FUN(spell_fear);
DECLARE_SPELL_FUN(spell_protection_heat);
DECLARE_SPELL_FUN(spell_protection_cold);
DECLARE_SPELL_FUN(spell_fire_shield);
DECLARE_SPELL_FUN(spell_witch_curse);
DECLARE_SPELL_FUN(spell_knock);
DECLARE_SPELL_FUN(spell_power_word_kill);
DECLARE_SPELL_FUN(spell_magic_resistance);
DECLARE_SPELL_FUN(spell_hallucination);
DECLARE_SPELL_FUN(spell_vampiric_blast);
DECLARE_SPELL_FUN(spell_dragon_skin);
DECLARE_SPELL_FUN(spell_mind_light);
DECLARE_SPELL_FUN(spell_lightning_shield);
DECLARE_SPELL_FUN(spell_shocking_trap);
DECLARE_SPELL_FUN(spell_insanity);
DECLARE_SPELL_FUN(spell_power_stun);
DECLARE_SPELL_FUN(spell_improved_detect);
DECLARE_SPELL_FUN(spell_improved_invis);
DECLARE_SPELL_FUN(spell_holy_armor);
DECLARE_SPELL_FUN(spell_bluefire);
DECLARE_SPELL_FUN(spell_severity_force);
DECLARE_SPELL_FUN(spell_bless_weapon);
DECLARE_SPELL_FUN(spell_resilience);
DECLARE_SPELL_FUN(spell_superior_heal);
DECLARE_SPELL_FUN(spell_master_healing);
DECLARE_SPELL_FUN(spell_group_heal);
DECLARE_SPELL_FUN(spell_restoring_light);
DECLARE_SPELL_FUN(spell_acid_arrow);
DECLARE_SPELL_FUN(spell_etheral_fist);
DECLARE_SPELL_FUN(spell_spectral_furor);
DECLARE_SPELL_FUN(spell_hand_of_undead);
DECLARE_SPELL_FUN(spell_disruption);
DECLARE_SPELL_FUN(spell_sonic_resonance);
DECLARE_SPELL_FUN(spell_mind_wrack);
DECLARE_SPELL_FUN(spell_mind_wrench);
DECLARE_SPELL_FUN(spell_sulfurus_spray);
DECLARE_SPELL_FUN(spell_caustic_font);
DECLARE_SPELL_FUN(spell_acetum_primus);
DECLARE_SPELL_FUN(spell_galvanic_whip);
DECLARE_SPELL_FUN(spell_magnetic_trust);
DECLARE_SPELL_FUN(spell_quantum_spike);
DECLARE_SPELL_FUN(spell_astral_walk);
DECLARE_SPELL_FUN(spell_mist_walk);
DECLARE_SPELL_FUN(spell_solar_flight);
DECLARE_SPELL_FUN(spell_helical_flow);
DECLARE_SPELL_FUN(spell_lesser_golem);
DECLARE_SPELL_FUN(spell_stone_golem);
DECLARE_SPELL_FUN(spell_iron_golem);
DECLARE_SPELL_FUN(spell_adamantite_golem);
DECLARE_SPELL_FUN(spell_corruption);
DECLARE_SPELL_FUN(spell_hurricane);
DECLARE_SPELL_FUN(spell_sanctify_lands);
DECLARE_SPELL_FUN(spell_deadly_venom);
DECLARE_SPELL_FUN(spell_cursed_lands);
DECLARE_SPELL_FUN(spell_lethargic_mist);
DECLARE_SPELL_FUN(spell_black_death);
DECLARE_SPELL_FUN(spell_mysterious_dream);
DECLARE_SPELL_FUN(spell_polymorph);
DECLARE_SPELL_FUN(spell_plant_form);
DECLARE_SPELL_FUN(spell_blade_barrier);
DECLARE_SPELL_FUN(spell_protection_negative);
DECLARE_SPELL_FUN(spell_ruler_aura);
DECLARE_SPELL_FUN(spell_evil_spirit);
DECLARE_SPELL_FUN(spell_disgrace);
DECLARE_SPELL_FUN(spell_control_undead);
DECLARE_SPELL_FUN(spell_assist);
DECLARE_SPELL_FUN(spell_aid);
DECLARE_SPELL_FUN(spell_summon_shadow);
DECLARE_SPELL_FUN(spell_remove_fear);
DECLARE_SPELL_FUN(spell_desert_fist);
DECLARE_SPELL_FUN(spell_take_revenge);
DECLARE_SPELL_FUN(spell_mirror);
DECLARE_SPELL_FUN(spell_doppelganger);
DECLARE_SPELL_FUN(spell_detect_fade);
DECLARE_SPELL_FUN(spell_shield_of_ruler);

#endif

