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
* $Id: gsn.h,v 1.26 1999-09-09 13:48:46 osya Exp $
*/

#ifndef _GSN_H_
#define _GSN_H_

extern namedp_t gsn_table[];

/*
* These are sn_lookup return values for common skills and spells.
*/

extern int gsn_backstab;
extern int gsn_dodge;
extern int gsn_envenom;
extern int gsn_hide;
extern int gsn_peek;
extern int gsn_sneak;
extern int gsn_disarm;
extern int gsn_enhanced_damage;
extern int gsn_kick;
extern int gsn_parry;
extern int gsn_rescue;
extern int gsn_second_attack;
extern int gsn_third_attack;
extern int gsn_blindness;
extern int gsn_charm_person;
extern int gsn_curse;
extern int gsn_remove_curse;
extern int gsn_invisibility;
extern int gsn_mass_invis;
extern int gsn_poison;
extern int gsn_plague;
extern int gsn_sleep;
extern int gsn_sanctuary;
extern int gsn_black_shroud;
extern int gsn_demand;
extern int gsn_fly;

extern int gsn_fourth_attack;
extern int gsn_dual_backstab;
extern int gsn_cleave;
extern int gsn_counter;
extern int gsn_ambush;
extern int gsn_circle;
extern int gsn_nerve;
extern int gsn_endure;
extern int gsn_quiet_movement;
extern int gsn_tame;
extern int gsn_assassinate;
extern int gsn_caltrops;
extern int gsn_throw;
extern int gsn_strangle;
extern int gsn_blackjack;
extern int gsn_bloodthirst;
extern int gsn_spellbane;
extern int gsn_resistance;
extern int gsn_deathblow;
extern int gsn_fade;
extern int gsn_garble;
extern int gsn_confuse;
extern int gsn_track;
extern int gsn_chaos_blade;
extern int gsn_wrath;
extern int gsn_stalker;
extern int gsn_tesseract;
extern int gsn_randomizer;
extern int gsn_trophy;
extern int gsn_truesight;
extern int gsn_brew;
extern int gsn_shadowlife;
extern int gsn_ruler_badge;
extern int gsn_remove_badge;
extern int gsn_dragon_strength;
extern int gsn_dragon_breath;
extern int gsn_warcry;
extern int gsn_dragonsword;
extern int gsn_guard;
extern int gsn_guard_call;
extern int gsn_love_potion;
extern int gsn_deafen;
extern int gsn_protective_shield;
extern int gsn_trance;
extern int gsn_demon_summon;
extern int gsn_nightwalker;
extern int gsn_squire;
extern int gsn_lightning_bolt;
extern int gsn_disperse;
extern int gsn_bless;
extern int gsn_weaken;
extern int gsn_haste;
extern int gsn_cure_critical;
extern int gsn_cure_serious;
extern int gsn_burning_hands;
extern int gsn_acid_blast;
extern int gsn_ray_of_truth;
extern int gsn_spell_craft;
extern int gsn_improved_maladiction;
extern int gsn_improved_benediction;
extern int gsn_improved_attack;
extern int gsn_improved_combat;
extern int gsn_improved_curative;
extern int gsn_improved_beguiling;
extern int gsn_improved_protective;
extern int gsn_giant_strength;
extern int gsn_explode;
extern int gsn_acid_breath;
extern int gsn_fire_breath;
extern int gsn_frost_breath;
extern int gsn_gas_breath;
extern int gsn_lightning_breath;
extern int gsn_cure_light;
extern int gsn_magic_missile;
extern int gsn_demonfire;
extern int gsn_faerie_fire;
extern int gsn_shield;
extern int gsn_chill_touch;
extern int gsn_second_weapon;
extern int gsn_target;
extern int gsn_sand_storm;
extern int gsn_scream;
extern int gsn_tiger_power;
extern int gsn_hara_kiri;
extern int gsn_enhanced_armor;
extern int gsn_vampire;
extern int gsn_vampiric_bite;
extern int gsn_light_resistance;
extern int gsn_blink;
extern int gsn_path_find;
extern int gsn_critical;
extern int gsn_detect_sneak;
extern int gsn_mend;
extern int gsn_shielding;
extern int gsn_blind_fighting;
extern int gsn_swimming;
extern int gsn_camouflage_move;
extern int gsn_protection_heat;
extern int gsn_protection_cold;
extern int gsn_teleport;
extern int gsn_witch_curse;
extern int gsn_kassandra;
extern int gsn_sebat;
extern int gsn_matandra;
extern int gsn_armor_use;
extern int gsn_cure_poison;
extern int gsn_fire_shield;
extern int gsn_fear;
extern int gsn_settraps;
extern int gsn_mental_attack;
extern int gsn_secondary_attack;
extern int gsn_mortal_strike;
extern int gsn_shield_cleave;
extern int gsn_weapon_cleave;
extern int gsn_slow;
extern int gsn_improved_invis;
extern int gsn_tail;
extern int gsn_power_word_stun;
extern int gsn_grip;
extern int gsn_concentrate;
extern int gsn_mastering_sword;
extern int gsn_master_hand;
extern int gsn_fifth_attack;
extern int gsn_area_attack;
extern int gsn_reserved;
extern int gsn_bandage;
extern int gsn_web;
extern int gsn_bow;
extern int gsn_bash_door;
extern int gsn_katana;
extern int gsn_bluefire;
extern int gsn_crush;
extern int gsn_charge;
extern int gsn_perception;
extern int gsn_deadly_venom;
extern int gsn_cursed_lands;
extern int gsn_lethargic_mist;
extern int gsn_black_death;
extern int gsn_mysterious_dream;
extern int gsn_sense_life;
extern int gsn_arrow;
extern int gsn_lance;
extern int gsn_evil_spirit;
extern int gsn_blindness_dust;
extern int gsn_poison_smoke;
extern int gsn_mastering_spell;
extern int gsn_lich;
extern int gsn_bone_dragon;

/* new_gsns */

extern int gsn_axe;
extern int gsn_dagger;
extern int gsn_flail;
extern int gsn_mace;
extern int gsn_polearm;
extern int gsn_shield_block;
extern int gsn_spear;
extern int gsn_sword;
extern int gsn_whip;
extern int gsn_staff;

extern int gsn_bash;
extern int gsn_berserk;
extern int gsn_dirt;
extern int gsn_hand_to_hand;
extern int gsn_trip;

extern int gsn_fast_healing;
extern int gsn_haggle;
extern int gsn_meditation;

extern int gsn_frenzy;
extern int gsn_riding;
extern int gsn_thumbling;
extern int gsn_pick;
extern int gsn_forest_fighting;
extern int gsn_mastering_bow;

extern int gsn_doppelganger;
extern int gsn_mirror;
extern int gsn_hand_block;
extern int gsn_knife;
extern int gsn_forest_blending;
extern int gsn_throw_weapon;
extern int gsn_herbs;
extern int gsn_fence;
extern int gsn_resurection;

extern int gsn_qtarget;

#endif
