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
 * $Id: spellfn.c,v 1.11 1999-02-23 07:55:42 kostik Exp $
 */

#include <stdio.h>

#include "typedef.h"
#include "namedp.h"
#include "spellfn.h"

NAMEDP spellfn_table[] =
{
	{ "spell_acid_blast",		spell_acid_blast		},
	{ "spell_armor",		spell_armor			},
	{ "spell_bless",		spell_bless			},
	{ "spell_blindness",		spell_blindness			},
	{ "spell_burning_hands",	spell_burning_hands		},
	{ "spell_call_lightning",	spell_call_lightning		},
	{ "spell_calm",			spell_calm			},
	{ "spell_cancellation",		spell_cancellation		},
	{ "spell_cause_critical",	spell_cause_critical		},
	{ "spell_cause_light",		spell_cause_light		},
	{ "spell_cause_serious",	spell_cause_serious		},
	{ "spell_healing_light",	spell_healing_light		},
	{ "spell_chain_lightning",	spell_chain_lightning		},
	{ "spell_charm_person",		spell_charm_person		},
	{ "spell_chill_touch",		spell_chill_touch		},
	{ "spell_colour_spray",		spell_colour_spray		},
	{ "spell_continual_light",	spell_continual_light		},
	{ "spell_control_weather",	spell_control_weather		},
	{ "spell_create_food",		spell_create_food		},
	{ "spell_create_rose",		spell_create_rose		},
	{ "spell_create_spring",	spell_create_spring		},
	{ "spell_create_water",		spell_create_water		},
	{ "spell_cure_blindness",	spell_cure_blindness		},
	{ "spell_cure_critical",	spell_cure_critical		},
	{ "spell_cure_disease",		spell_cure_disease		},
	{ "spell_cure_light",		spell_cure_light		},
	{ "spell_cure_poison",		spell_cure_poison		},
	{ "spell_cure_serious",		spell_cure_serious		},
	{ "spell_curse",		spell_curse			},
	{ "spell_anathema",		spell_anathema			},
	{ "spell_demonfire",		spell_demonfire			},
	{ "spell_detect_evil",		spell_detect_evil		},
	{ "spell_detect_good",		spell_detect_good		},
	{ "spell_detect_hidden",	spell_detect_hidden		},
	{ "spell_detect_invis",		spell_detect_invis		},
	{ "spell_detect_magic",		spell_detect_magic		},
	{ "spell_detect_poison",	spell_detect_poison		},
	{ "spell_detect_undead",	spell_detect_undead		},
	{ "spell_disintegrate",		spell_disintegrate		},
	{ "spell_dispel_evil",		spell_dispel_evil		},
	{ "spell_dispel_good",		spell_dispel_good		},
	{ "spell_dispel_magic",		spell_dispel_magic		},
	{ "spell_earthquake",		spell_earthquake		},
	{ "spell_enchant_armor",	spell_enchant_armor		},
	{ "spell_enchant_weapon",	spell_enchant_weapon		},
	{ "spell_hunger_weapon",	spell_hunger_weapon		},
	{ "spell_energy_drain",		spell_energy_drain		},
	{ "spell_faerie_fire",		spell_faerie_fire		},
	{ "spell_faerie_fog",		spell_faerie_fog		},
	{ "spell_farsight",		spell_farsight			},
	{ "spell_fireball",		spell_fireball			},
	{ "spell_fireproof",		spell_fireproof			},
	{ "spell_flamestrike",		spell_flamestrike		},
	{ "spell_floating_disc",	spell_floating_disc		},
	{ "spell_fly",			spell_fly			},
	{ "spell_frenzy",		spell_frenzy			},
	{ "spell_gate",			spell_gate			},
	{ "spell_giant_strength",	spell_giant_strength		},
	{ "spell_harm",			spell_harm			},
	{ "spell_haste",		spell_haste			},
	{ "spell_heal",			spell_heal			},
	{ "spell_heat_metal",		spell_heat_metal		},
	{ "spell_holy_word",		spell_holy_word			},
	{ "spell_identify",		spell_identify			},
	{ "spell_infravision",		spell_infravision		},
	{ "spell_invisibility",		spell_invisibility		},
	{ "spell_know_alignment",	spell_know_alignment		},
	{ "spell_lightning_bolt",	spell_lightning_bolt		},
	{ "spell_locate_object",	spell_locate_object		},
	{ "spell_magic_missile",	spell_magic_missile		},
	{ "spell_mass_healing",		spell_mass_healing		},
	{ "spell_mass_invis",		spell_mass_invis		},
	{ "spell_nexus",		spell_nexus			},
	{ "spell_pass_door",		spell_pass_door			},
	{ "spell_plague",		spell_plague			},
	{ "spell_poison",		spell_poison			},
	{ "spell_portal",		spell_portal			},
	{ "spell_protection_evil",	spell_protection_evil		},
	{ "spell_protection_good",	spell_protection_good		},
	{ "spell_ray_of_truth",		spell_ray_of_truth		},
	{ "spell_recharge",		spell_recharge			},
	{ "spell_refresh",		spell_refresh			},
	{ "spell_remove_curse",		spell_remove_curse		},
	{ "spell_sanctuary",		spell_sanctuary			},
	{ "spell_black_shroud",		spell_black_shroud		},
	{ "spell_shocking_grasp",	spell_shocking_grasp		},
	{ "spell_shield",		spell_shield			},
	{ "spell_sleep",		spell_sleep			},
	{ "spell_slow",			spell_slow			},
	{ "spell_stone_skin",		spell_stone_skin		},
	{ "spell_summon",		spell_summon			},
	{ "spell_teleport",		spell_teleport			},
	{ "spell_ventriloquate",	spell_ventriloquate		},
	{ "spell_weaken",		spell_weaken			},
	{ "spell_word_of_recall",	spell_word_of_recall		},
	{ "spell_acid_breath",		spell_acid_breath		},
	{ "spell_fire_breath",		spell_fire_breath		},
	{ "spell_frost_breath",		spell_frost_breath		},
	{ "spell_gas_breath",		spell_gas_breath		},
	{ "spell_lightning_breath",	spell_lightning_breath		},
	{ "spell_general_purpose",	spell_general_purpose		},
	{ "spell_high_explosive",	spell_high_explosive		},
	{ "spell_mana_transfer",	spell_mana_transfer		},
	{ "spell_transform",		spell_transform			},
	{ "spell_demon_summon",		spell_demon_summon		},
	{ "spell_scourge",		spell_scourge			},
	{ "spell_iceball",		spell_iceball			},
	{ "spell_manacles",		spell_manacles			},
	{ "spell_mental_knife",		spell_mental_knife		},
	{ "spell_guard_call",		spell_guard_call		},
	{ "spell_nightwalker",		spell_nightwalker		},
	{ "spell_eyes_of_intrigue",	spell_eyes_of_intrigue		},
	{ "spell_shadow_cloak",		spell_shadow_cloak		},
	{ "spell_nightfall",		spell_nightfall			},
	{ "spell_confuse",		spell_confuse			},
	{ "spell_garble",		spell_garble			},
	{ "spell_terangreal",		spell_terangreal		},
	{ "spell_kassandra",		spell_kassandra			},
	{ "spell_matandra",		spell_matandra			},
	{ "spell_sebat",		spell_sebat			},
	{ "spell_amnesia",		spell_amnesia			},
	{ "spell_chaos_blade",		spell_chaos_blade		},
	{ "spell_tattoo",		spell_tattoo			},
	{ "spell_remove_tattoo",	spell_remove_tattoo		},
	{ "spell_wrath",		spell_wrath			},
	{ "spell_randomizer",		spell_randomizer		},
	{ "spell_tesseract",		spell_tesseract			},
	{ "spell_stalker",		spell_stalker			},
	{ "spell_brew",			spell_brew			},
	{ "spell_shadowlife",		spell_shadowlife		},
	{ "spell_ruler_badge",		spell_ruler_badge		},
	{ "spell_remove_badge",		spell_remove_badge		},
	{ "spell_dragon_strength",	spell_dragon_strength		},
	{ "spell_dragon_breath",	spell_dragon_breath		},
	{ "spell_golden_aura",		spell_golden_aura		},
	{ "spell_dragonplate",		spell_dragonplate		},
	{ "spell_squire",		spell_squire			},
	{ "spell_dragonsword",		spell_dragonsword		},
	{ "spell_entangle",		spell_entangle			},
	{ "spell_love_potion",		spell_love_potion		},
	{ "spell_deafen",		spell_deafen			},
	{ "spell_protective_shield",	spell_protective_shield		},
	{ "spell_disperse",		spell_disperse			},
	{ "spell_bark_skin",		spell_bark_skin			},
	{ "spell_acute_vision",		spell_acute_vision		},
	{ "spell_ranger_staff",		spell_ranger_staff		},
	{ "spell_hellfire",		spell_hellfire			},
	{ "spell_dragons_breath",	spell_dragons_breath		},
	{ "spell_sand_storm",		spell_sand_storm		},
	{ "spell_scream",		spell_scream			},
	{ "spell_attract_other",	spell_attract_other		},
	{ "spell_animate_dead",		spell_animate_dead		},
	{ "spell_enhanced_armor",	spell_enhanced_armor		},
	{ "spell_meld_into_stone",	spell_meld_into_stone		},
	{ "spell_find_object",		spell_find_object		},
	{ "spell_shielding",		spell_shielding			},
	{ "spell_web",			spell_web			},
	{ "spell_group_defense",	spell_group_defense		},
	{ "spell_inspire",		spell_inspire			},
	{ "spell_link",			spell_link			},
	{ "spell_mass_sanctuary",	spell_mass_sanctuary		},
	{ "spell_mend",			spell_mend			},
	{ "spell_wolf",			spell_wolf			},
	{ "spell_lion_help",		spell_lion_help			},
	{ "spell_magic_jar",		spell_magic_jar			},
	{ "spell_turn",			spell_turn			},
	{ "spell_fear",			spell_fear			},
	{ "spell_protection_heat",	spell_protection_heat		},
	{ "spell_protection_cold",	spell_protection_cold		},
	{ "spell_fire_shield",		spell_fire_shield		},
	{ "spell_witch_curse",		spell_witch_curse		},
	{ "spell_knock",		spell_knock			},
	{ "spell_power_word_kill",	spell_power_word_kill		},
	{ "spell_magic_resistance",	spell_magic_resistance		},
	{ "spell_hallucination",	spell_hallucination		},
	{ "spell_vampiric_blast",	spell_vampiric_blast		},
	{ "spell_dragon_skin",		spell_dragon_skin		},
	{ "spell_mind_light",		spell_mind_light		},
	{ "spell_lightning_shield",	spell_lightning_shield		},
	{ "spell_shocking_trap",	spell_shocking_trap		},
	{ "spell_insanity",		spell_insanity			},
	{ "spell_power_stun",		spell_power_stun		},
	{ "spell_improved_detect",	spell_improved_detect		},
	{ "spell_improved_invis",	spell_improved_invis		},
	{ "spell_holy_armor",		spell_holy_armor		},
	{ "spell_bluefire",		spell_bluefire			},
	{ "spell_severity_force",	spell_severity_force		},
	{ "spell_bless_weapon",		spell_bless_weapon		},
	{ "spell_resilience",		spell_resilience		},
	{ "spell_superior_heal",	spell_superior_heal		},
	{ "spell_master_healing",	spell_master_healing		},
	{ "spell_group_heal",		spell_group_heal		},
	{ "spell_restoring_light",	spell_restoring_light		},
	{ "spell_acid_arrow",		spell_acid_arrow		},
	{ "spell_etheral_fist",		spell_etheral_fist		},
	{ "spell_spectral_furor",	spell_spectral_furor		},
	{ "spell_hand_of_undead",	spell_hand_of_undead		},
	{ "spell_disruption",		spell_disruption		},
	{ "spell_sonic_resonance",	spell_sonic_resonance		},
	{ "spell_mind_wrack",		spell_mind_wrack		},
	{ "spell_mind_wrench",		spell_mind_wrench		},
	{ "spell_sulfurus_spray",	spell_sulfurus_spray		},
	{ "spell_caustic_font",		spell_caustic_font		},
	{ "spell_acetum_primus",	spell_acetum_primus		},
	{ "spell_galvanic_whip",	spell_galvanic_whip		},
	{ "spell_magnetic_trust",	spell_magnetic_trust		},
	{ "spell_quantum_spike",	spell_quantum_spike		},
	{ "spell_astral_walk",		spell_astral_walk		},
	{ "spell_mist_walk",		spell_mist_walk			},
	{ "spell_solar_flight",		spell_solar_flight		},
	{ "spell_helical_flow",		spell_helical_flow		},
	{ "spell_lesser_golem",		spell_lesser_golem		},
	{ "spell_stone_golem",		spell_stone_golem		},
	{ "spell_iron_golem",		spell_iron_golem		},
	{ "spell_adamantite_golem",	spell_adamantite_golem		},
	{ "spell_corruption",		spell_corruption		},
	{ "spell_hurricane",		spell_hurricane			},
	{ "spell_sanctify_lands",	spell_sanctify_lands		},
	{ "spell_deadly_venom",		spell_deadly_venom		},
	{ "spell_cursed_lands",		spell_cursed_lands		},
	{ "spell_lethargic_mist",	spell_lethargic_mist		},
	{ "spell_black_death",		spell_black_death		},
	{ "spell_mysterious_dream",	spell_mysterious_dream		},
	{ "spell_polymorph",		spell_polymorph			},
	{ "spell_plant_form",		spell_plant_form		},
	{ "spell_blade_barrier",	spell_blade_barrier		},
	{ "spell_protection_negative",	spell_protection_negative	},
	{ "spell_ruler_aura",		spell_ruler_aura		},
	{ "spell_evil_spirit",		spell_evil_spirit		},
	{ "spell_disgrace",		spell_disgrace			},
	{ "spell_control_undead",	spell_control_undead		},
	{ "spell_assist",		spell_assist			},
	{ "spell_aid",			spell_aid			},
	{ "spell_summon_shadow",	spell_summon_shadow		},
	{ "spell_remove_fear",		spell_remove_fear		},
	{ "spell_desert_fist",		spell_desert_fist		},
	{ "spell_take_revenge",		spell_take_revenge		},
	{ "spell_mirror",		spell_mirror			},
	{ "spell_doppelganger",		spell_doppelganger		},
	{ "spell_detect_fade",		spell_detect_fade		},
	{ "spell_shield_of_ruler",	spell_shield_of_ruler		},
	{ NULL }
};

