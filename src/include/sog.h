/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: sog.h,v 1.4 2001-07-31 18:14:33 fjoe Exp $
 */

#ifndef _HANDLER_H_
#define _HANDLER_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_HANDLER
#include <dynafun_decl.h>

__MODULE_START_DECL

/*--- handler.c */
DECLARE_PROC2(char_to_room,
	      ARG(CHAR_DATA), ch, ARG(ROOM_INDEX_DATA), pRoomIndex)
DECLARE_PROC1(char_from_room,
	      ARG(CHAR_DATA), ch)

DECLARE_PROC2(obj_to_char,
	      ARG(OBJ_DATA), obj, ARG(CHAR_DATA), ch)
DECLARE_PROC1(obj_from_char,
	      ARG(OBJ_DATA), obj)

DECLARE_PROC2(obj_to_room,
	      ARG(OBJ_DATA), obj, ARG(ROOM_INDEX_DATA), pRoomIndex)
DECLARE_PROC1(obj_from_room,
	      ARG(OBJ_DATA), obj)

DECLARE_PROC2(obj_to_obj,
	      ARG(OBJ_DATA), obj, ARG(OBJ_DATA), obj_to)
DECLARE_PROC1(obj_from_obj,
	      ARG(OBJ_DATA), obj)


/* extract obj flags */
#define XO_F_NOCOUNT	(A)	/* do not update obj count		*/
#define XO_F_NORECURSE	(B)	/* do not extract contained in objs	*/
#define XO_F_NOCHQUEST	(C)	/* do not check for chquest objs	*/

/* create mob flags */
#define CM_F_NOLIST	(A)	/* do not insert in char_list */

/* quit_char/extract_char flags */
#define XC_F_NOCOUNT	(A)	/* update obj count			*/
#define XC_F_INCOMPLETE	(B)	/* do not extract char from char_list	*/

DECLARE_FUN2(CHAR_DATA, create_mob,
	     ARG(MOB_INDEX_DATA), pMobIndex, ARG(int), flags)
DECLARE_FUN2(CHAR_DATA, create_mob_of,
	     ARG(MOB_INDEX_DATA), pMobIndex, ARG(mlstring), owner)
DECLARE_FUN1(CHAR_DATA, clone_mob,
	     ARG(CHAR_DATA), parent)
DECLARE_PROC2(extract_char,
	      ARG(CHAR_DATA), ch, ARG(int), flags)
DECLARE_PROC2(quit_char,
	      ARG(CHAR_DATA), ch, ARG(int), flags)

/* create_obj flags */
#define CO_F_NOCOUNT	(A)	/* do not update obj count */

DECLARE_FUN2(OBJ_DATA, create_obj,
	     ARG(OBJ_INDEX_DATA), pObjIndex, ARG(int), flags)
DECLARE_FUN2(OBJ_DATA, create_obj_of,
	     ARG(OBJ_INDEX_DATA), pObjIndex, ARG(mlstring), owner)
DECLARE_FUN1(OBJ_DATA,	clone_obj,
	     ARG(OBJ_DATA), parent)

DECLARE_PROC2(extract_obj,
	      ARG(OBJ_DATA), obj, ARG(int), flags)

DECLARE_FUN2(bool, can_see,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN2(bool, can_see_obj,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_FUN2(bool, can_see_room,
	     ARG(CHAR_DATA), ch, ARG(ROOM_INDEX_DATA), pRoomIndex)

DECLARE_FUN1(bool, check_blind,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, check_blind_nomessage,
	     ARG(CHAR_DATA), ch)

DECLARE_FUN2(CHAR_DATA, nth_char,
	     ARG(CHAR_DATA), ch, ARG(int), n)
DECLARE_FUN2(OBJ_DATA, nth_obj,
	     ARG(OBJ_DATA), obj, ARG(int), n)

DECLARE_FUN1(int, can_carry_n,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, can_carry_w,
	     ARG(CHAR_DATA), ch)

DECLARE_FUN1(int, get_obj_number,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, get_obj_realnumber,
	     ARG(OBJ_DATA), obj)

DECLARE_FUN1(int, get_obj_weight,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, get_obj_realweight,
	     ARG(OBJ_DATA), obj)

DECLARE_PROC2(_equip_char,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_FUN3(OBJ_DATA, equip_char,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj, ARG(int), iWear)
DECLARE_PROC2(unequip_char,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_FUN2(OBJ_DATA, get_eq_char,
	     ARG(CHAR_DATA), ch, ARG(int), iWear)

/*
 * move_char flags
 */
#define MC_F_CHARGE	(A)

DECLARE_FUN3(bool, move_char,
	     ARG(CHAR_DATA), ch, ARG(int), door, ARG(flag_t), flags)

DECLARE_FUN1(int, get_hours,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, get_age,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(int, get_resist,
	     ARG(CHAR_DATA), ch, ARG(int), dam_class)

DECLARE_FUN2(CHAR_DATA, get_char_room,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(CHAR_DATA, get_char_area,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(CHAR_DATA, get_char_world,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)

DECLARE_FUN1(int, exp_to_level,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(int, exp_for_level,
	     ARG(CHAR_DATA), ch, ARG(int), level)
DECLARE_PROC2(gain_exp,
	      ARG(CHAR_DATA), ch, ARG(int), gain)
DECLARE_PROC1(advance_level,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC1(delevel,
	      ARG(CHAR_DATA), ch)

DECLARE_FUN1(bool, room_is_dark,
	     ARG(ROOM_INDEX_DATA), pRoomIndex)
DECLARE_FUN1(bool, room_is_private,
	     ARG(ROOM_INDEX_DATA), pRoomIndex)
DECLARE_FUN2(ROOM_INDEX_DATA, find_location,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)

DECLARE_PROC4(get_obj,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj, ARG(OBJ_DATA), container,
	      ARG(cchar_t), msg_others)
DECLARE_PROC2(quaff_obj,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_PROC3(wear_obj,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj, ARG(bool), fReplace)
DECLARE_FUN3(bool, remove_obj,
	     ARG(CHAR_DATA), ch, ARG(int), iWear, ARG(bool), fReplace)

DECLARE_FUN4(OBJ_DATA, get_obj_list,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument, ARG(OBJ_DATA), list,
	     ARG(int), flags)
DECLARE_FUN2(OBJ_DATA, get_obj_carry,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(OBJ_DATA, get_obj_wear,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(OBJ_DATA, get_obj_here,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(OBJ_DATA, get_obj_room,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(OBJ_DATA, get_obj_world,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument)

DECLARE_PROC2(add_mind,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), str)
DECLARE_PROC2(remove_mind,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), str)
DECLARE_PROC1(back_home,
	      ARG(CHAR_DATA), ch)

DECLARE_FUN2(bool, is_leader,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), lch)
DECLARE_PROC2(set_leader,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), lch)
DECLARE_FUN1(CHAR_DATA, leader_lookup,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, is_same_group,
	     ARG(CHAR_DATA), ach, ARG(CHAR_DATA), bch)

DECLARE_PROC2(add_follower,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), master)
DECLARE_PROC1(stop_follower,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC1(die_follower,
	      ARG(CHAR_DATA), ch)

DECLARE_PROC4(damage_to_obj,
	      ARG(CHAR_DATA), ch, ARG(OBJ_DATA), wield, ARG(OBJ_DATA), worn,
	      ARG(int), dam)
DECLARE_FUN4(bool, make_eq_damage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(int), loc_wield,
	     ARG(int), loc_destroy)
DECLARE_FUN3(bool, random_eq_damage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(int), loc_wield)

DECLARE_FUN2(int, need_hands,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), weapon)
DECLARE_FUN1(int, free_hands,
	     ARG(CHAR_DATA), ch)

DECLARE_FUN1(int, exit_lookup,
	     ARG(cchar_t), arg)
DECLARE_FUN2(int, find_door_nomessage,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), arg)
DECLARE_FUN2(int, find_door,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), arg)

DECLARE_PROC6(transfer_char,
	      ARG(CHAR_DATA), ch,
	      NULLABLE_ARG(CHAR_DATA), vch,
	      ARG(ROOM_INDEX_DATA), to_room,
	      NULLABLE_ARG(cchar_t), msg_out,
	      NULLABLE_ARG(cchar_t), msg_travel,
	      NULLABLE_ARG(cchar_t), msg_in)
DECLARE_PROC2(recall,
	      ARG(CHAR_DATA), ch, ARG(ROOM_INDEX_DATA), room)

DECLARE_FUN2(bool, can_gate,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN2(bool, can_drop_obj,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_FUN2(bool, can_loot,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)

DECLARE_FUN1(bool, pc_name_ok,
	     ARG(cchar_t), name)
DECLARE_FUN1(bool, char_in_dark_room,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC3(do_who_raw,
	      NULLABLE_ARG(CHAR_DATA), ch, ARG(CHAR_DATA), vch,
	      ARG(BUFFER), output)
DECLARE_FUN1(int, isn_dark_safe,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC2(format_obj,
	      ARG(BUFFER), output, ARG(OBJ_DATA), obj)
DECLARE_FUN2(int, count_obj_list,
	     ARG(OBJ_INDEX_DATA), obj, NULLABLE_ARG(OBJ_DATA), list)
DECLARE_PROC2(label_add,
	      ARG(OBJ_DATA), obj, ARG(cchar_t), name)
DECLARE_FUN2(OBJ_DATA, create_money,
	     ARG(int), gold, ARG(int), silver)
DECLARE_FUN1(int, age_to_num,
	     ARG(int), age)
DECLARE_PROC2(make_visible,
	      ARG(CHAR_DATA), ch, ARG(bool), at_will)
DECLARE_PROC2(bad_effect,
	      ARG(CHAR_DATA), ch, ARG(int), effect)
DECLARE_FUN1(int, trust_level,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(cchar_t, get_cond_alias,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN3(bool, mount_success,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), mount, ARG(int), canattack)
DECLARE_FUN2(cchar_t, get_stat_alias,
	     ARG(CHAR_DATA), ch, ARG(int), st)
DECLARE_PROC2(set_title,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_PROC2(look_at,
	      ARG(CHAR_DATA), ch, ARG(ROOM_INDEX_DATA), room)
DECLARE_FUN2(ROOM_INDEX_DATA, get_random_room,
	     NULLABLE_ARG(CHAR_DATA), ch, NULLABLE_ARG(AREA_DATA), area)
DECLARE_PROC2(deduct_cost,
	      ARG(CHAR_DATA), ch, ARG(int), cost)
DECLARE_FUN1(int, count_charmed,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC3(path_to_track,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(int), door)
DECLARE_PROC3(room_record,
	      ARG(cchar_t), name, ARG(ROOM_INDEX_DATA), room, ARG(int), door)
DECLARE_FUN1(int, count_users,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN3(int, apply_ac,
	     ARG(OBJ_DATA), obj, ARG(int), iWear, ARG(int), type)
DECLARE_FUN4(CHAR_DATA,	find_char,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument, ARG(int), door,
	     ARG(int), range)
DECLARE_FUN4(CHAR_DATA, get_char_spell,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), argument, ARG(pvoid_t), door,
	     ARG(int), range)

DECLARE_FUN2(bool, shapeshift,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), form)
DECLARE_FUN1(bool, revert,
	     ARG(CHAR_DATA), ch)

/*--- scan_pfiles.c */
DECLARE_PROC0(scan_pfiles)

/*--- reset.c */

/* reset_room flags */
#define RESET_F_NOPCHECK (A)

DECLARE_PROC2(reset_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(int), flags)
DECLARE_PROC1(reset_area,
	      ARG(AREA_DATA), pArea)

/*--- effects.c */
DECLARE_PROC3(acid_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(cold_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(fire_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(poison_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(shock_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(sand_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)
DECLARE_PROC3(scream_effect,
	      ARG(pvoid_t), vo, ARG(int), level, ARG(int), dam)

/*--- affects.c */
DECLARE_PROC3(affect_modify,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf, ARG(bool), fAdd)
DECLARE_PROC3(affect_modify_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf,
	      ARG(bool), fAdd)
DECLARE_FUN2(AFFECT_DATA, affect_find,
	     NULLABLE_ARG(AFFECT_DATA), paf, ARG(cchar_t), sn)
DECLARE_PROC3(affect_check,
	      ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), vector)
DECLARE_PROC4(affect_check_list,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf, ARG(int), where,
	      ARG(flag_t), vector)
DECLARE_PROC1(affect_enchant,
	      ARG(OBJ_DATA), obj)

DECLARE_PROC2(affect_to_char,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_to_obj,
	      ARG(OBJ_DATA), obj, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove_obj,
	      ARG(OBJ_DATA), obj, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_strip,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_PROC3(affect_bit_strip,
	      ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), bits)
DECLARE_PROC2(affect_join,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)

DECLARE_FUN2(bool, is_affected,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN3(AFFECT_DATA, is_bit_affected,
	     ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), bits)
DECLARE_FUN2(bool, has_obj_affect,
	     ARG(CHAR_DATA), ch, ARG(flag_t), vector)

/* room affects */
DECLARE_PROC2(affect_to_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_strip_room,
	      ARG(ROOM_INDEX_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN2(bool, is_affected_room,
	     ARG(ROOM_INDEX_DATA), ch, ARG(cchar_t), sn)
DECLARE_PROC1(strip_raff_owner,
	      ARG(CHAR_DATA), ch)

DECLARE_PROC2(aff_dump_list,
	      ARG(AFFECT_DATA), paf, ARG(BUFFER), output)
DECLARE_PROC3(show_affects,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), vch, ARG(BUFFER), output)

/* format_obj_affects flags */
#define FOA_F_NODURATION	(A)	/* do not show duration		*/
#define FOA_F_NOAFFECTS		(B)	/* do not show bit affects	*/

DECLARE_PROC3(format_obj_affects,
	      ARG(BUFFER), output, ARG(AFFECT_DATA), paf, ARG(int), flags)

/*--- fight.c */

/*
 * damage() flags
 */
#define DAMF_NONE	(0)
#define DAMF_SHOW	(A)	/* show dam message */
#define DAMF_SECOND	(B)	/* damage inflicted by second weapon */
#define DAMF_HIT	(C)	/* damage by hit */
#define DAMF_HUNGER	(D)	/* damage by hunger */
#define DAMF_NOREDUCE	(E)	/* damage should not be reduced */
#define DAMF_THIRST	(F)	/* damage by thirst */
#define DAMF_LIGHT_V	(G)	/* light in room */
#define DAMF_TRAP_ROOM	(H)	/* room trap damage */

DECLARE_PROC4(one_hit,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), dt,
	      ARG(bool), secondary)
DECLARE_PROC2(handle_death,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_PROC3(multi_hit,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), dt)
DECLARE_FUN6(bool, damage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(int), dam,
	     ARG(cchar_t), dt, ARG(int), class, ARG(int), dam_flags)
DECLARE_PROC1(update_pos,
	      ARG(CHAR_DATA), victim)
DECLARE_PROC2(set_fighting,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_PROC2(stop_fighting,
	      ARG(CHAR_DATA), ch, ARG(bool), fBoth)
DECLARE_FUN2(OBJ_DATA, raw_kill,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN1(cchar_t, vs_dam_alias,
	     ARG(int), dam)
DECLARE_FUN1(cchar_t, vp_dam_alias,
	     ARG(int), dam)
DECLARE_FUN4(bool, check_obj_dodge,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim,
	     ARG(OBJ_DATA), obj, ARG(int), bonus)
DECLARE_FUN2(bool, is_safe,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN2(bool, is_safe_nomessage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN3(bool, is_safe_spell,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(bool), area)
DECLARE_FUN2(bool, is_safe_rspell,
	     ARG(AFFECT_DATA), af, ARG(CHAR_DATA), victim)
DECLARE_FUN2(int, get_dam_class,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), wield)
DECLARE_FUN2(CHAR_DATA, check_guard,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), mob)
DECLARE_FUN2(bool, in_PK,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)

DECLARE_PROC4(focus_positive_energy,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), sn,
	      ARG(int), amount)
DECLARE_PROC4(focus_negative_energy,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), sn,
	      ARG(int), amount)

/*--- skills.c */
DECLARE_FUN2(int, skill_level,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN1(int, skill_beats,
	     ARG(cchar_t), sn)
DECLARE_FUN2(int, skill_mana,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN1(gmlstr_t, skill_noun,
	     ARG(cchar_t), sn)

DECLARE_FUN2(int, get_skill,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_PROC3(set_skill,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn, ARG(int), value)
DECLARE_PROC4(_set_skill,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn, ARG(int), value,
	      ARG(bool), repl)

DECLARE_PROC4(check_improve,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn,
	      ARG(bool), success, ARG(int), mult)

DECLARE_FUN1(cchar_t, get_weapon_sn,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN2(int, get_weapon_skill,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)

DECLARE_PROC2(skills_dump,
	      ARG(BUFFER), output, ARG(int), skill_type)
DECLARE_PROC2(say_spell,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn)

DECLARE_PROC3(check_one_event,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), af, ARG(flag_t), event)
DECLARE_PROC3(check_events,
	      ARG(CHAR_DATA), ch, NULLABLE_ARG(AFFECT_DATA), list,
	      ARG(flag_t), event)

/*--- spec.c */
DECLARE_PROC2(spec_stats,
	      ARG(CHAR_DATA), ch, ARG(spec_skill_t), spec_sk)

DECLARE_PROC1(update_skills,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC1(spec_update,
	      ARG(CHAR_DATA), ch)

DECLARE_FUN2(bool, has_spec,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn)
DECLARE_FUN2(bool, spec_add,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn)
DECLARE_FUN2(bool, spec_del,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn)
DECLARE_FUN3(cchar_t, spec_replace,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn_rm, ARG(cchar_t), spn_add)

__MODULE_END_DECL

/*
 * get_obj_list flags
 */
enum {
	GETOBJ_F_ANY,		/* any obj->wear_loc			     */
	GETOBJ_F_INV,		/* obj->wear_loc == WEAR_NONE (in inventory) */
	GETOBJ_F_WORN,		/* obj->wear_loc != WEAR_NONE (worn)	     */
};

#endif /* _HANDLER_H_ */
