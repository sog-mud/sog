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
 * $Id: init_mpc.c,v 1.50 2002-11-23 15:27:56 fjoe Exp $
 */

#include <dlfcn.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <merc.h>
#include <dynafun.h>
#include <module.h>
#include <rwfile.h>
#include <mprog.h>

#include "mpc_impl.h"
#include "mpc_const.h"

#if !defined(MPC)
#define MODULE_INIT MOD_MPC
#include "mpc_dynafun.h"

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

#endif

static dynafun_data_t core_dynafun_tab[] = {
	DECLARE_FUN2(int, dice,
		     ARG(int), number, ARG(int), size)
	DECLARE_PROC3(dofun,
		      ARG(cchar_t), name, ARG(CHAR_DATA), ch,
		      ARG(cchar_t), argument)
	DECLARE_FUN1(ROOM_INDEX_DATA, get_room_index,
		     ARG(int), vnum)
	DECLARE_FUN1(int, number_bits,
		     ARG(int), width)
	DECLARE_FUN0(int, number_percent)
	DECLARE_FUN2(int, number_range,
		     ARG(int), from, ARG(int), to)
#if defined(MPC)
	{ "print",		MT_VOID, 1,
	  { { MT_INT } }						},
	{ "print2",		MT_VOID, 2,
	  { { MT_INT }, { MT_INT } }					},
	{ "prints",		MT_VOID, 1,
	  { { MT_STR } }						},
	{ "nonexistent",	MT_VOID, 0				},
#endif
	NULL_DYNAFUN_DATA
};

#if !defined(MPC)
avltree_t mpcodes;

static varr_info_t c_info_swjumps = {
	&varr_ops, NULL, NULL,

	sizeof(swjump_t), 4
};

static void
jumptab_init(varr *v)
{
	c_init(v, &c_info_swjumps);
}

static void
jumptab_destroy(varr *v)
{
	c_destroy(v);
}

static void
svar_init(svar_t *svar)
{
	svar->name = str_empty;
	svar->type_tag = 0;
	svar->var_flags = 0;
}

static avltree_info_t c_info_svars = {
	&avltree_ops,

	(e_init_t) svar_init,
	strkey_destroy,

	MT_PVOID, sizeof(svar_t), ke_cmp_str
};

static void
svh_init(svh_t *svh)
{
	svh->sym_name = str_empty;
	c_init(&svh->svars, &c_info_svars);
}

static void
svh_destroy(svh_t *svh)
{
	free_string(svh->sym_name);
	c_destroy(&svh->svars);
}

static varr_info_t c_info_ints = {
	&varr_ops, NULL, NULL,

	sizeof(int), 8
};

static varr_info_t c_info_jumptabs = {
	&varr_ops,

	(e_init_t) jumptab_init,
	(e_destroy_t) jumptab_destroy,

	sizeof(varr), 4
};

static varr_info_t c_info_iters = {
	&varr_ops, NULL, NULL,

	sizeof(iterdata_t), 4
};

static varr_info_t c_info_vos = {
	&varr_ops, NULL, NULL,

	sizeof(vo_t), 4
};

static avltree_info_t c_info_strings = {
	&avltree_ops,

	strkey_init,
	strkey_destroy,

	MT_PVOID, sizeof(char *), ke_cmp_csstr,
};

static avltree_info_t c_info_svhs = {
	&avltree_ops,

	(e_init_t) svh_init,
	(e_destroy_t) svh_destroy,

	MT_PVOID, sizeof(svh_t), ke_cmp_str
};

static void
mpcode_init(mpcode_t *mpc)
{
	mpc->name = str_empty;
	mpc->mp = NULL;
	mpc->lineno = 0;

	c_init(&mpc->strings, &c_info_strings);
	c_init(&mpc->syms, &c_info_syms);
	c_init(&mpc->svhs, &c_info_svhs);

	c_init(&mpc->cstack, &c_info_ints);
	c_init(&mpc->args, &c_info_ints);
	mpc->curr_block = 0;

	mpc->curr_jumptab = -1;
	mpc->curr_break_addr = INVALID_ADDR;
	mpc->curr_continue_addr = INVALID_ADDR;

	mpc->retval = 0;
	mpc->ip = 0;
	c_init(&mpc->code, &c_info_ints);

	c_init(&mpc->jumptabs, &c_info_jumptabs);
	c_init(&mpc->iters, &c_info_iters);

	c_init(&mpc->data, &c_info_vos);
}

static void
mpcode_destroy(mpcode_t *mpc)
{
	free_string(mpc->name);

	c_destroy(&mpc->strings);
	c_destroy(&mpc->syms);
	c_destroy(&mpc->svhs);

	c_destroy(&mpc->cstack);
	c_destroy(&mpc->args);

	c_destroy(&mpc->code);

	c_destroy(&mpc->jumptabs);
	c_destroy(&mpc->iters);

	c_destroy(&mpc->data);
}

avltree_info_t c_info_mpcodes = {
	&avltree_ops,

	(e_init_t) mpcode_init,
	(e_destroy_t) mpcode_destroy,

	MT_PVOID, sizeof(mpcode_t), ke_cmp_csstr
};

MODINIT_FUN(_module_load, m)
{
	mprog_t *mp;

	mprog_compile = dlsym(m->dlh, "_mprog_compile");	// notrans
	if (mprog_compile == NULL) {
		log(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	mprog_execute = dlsym(m->dlh, "_mprog_execute");	// notrans
	if (mprog_execute == NULL) {
		log(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	if (mpc_init() < 0)
		return -1;

	dynafun_tab_register(__mod_tab(MODULE), m);

	c_init(&mpcodes, &c_info_mpcodes);
	C_FOREACH(mp, &mprogs) {
		if (mprog_compile(mp) < 0) {
			log(LOG_INFO, "load_mprog: %s (%s)",
			    mp->name, flag_string(mprog_types, mp->type));
			fprintf(stderr, "%s", buf_string(mp->errbuf));
		}
	}

	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	mprog_compile = NULL;
	mprog_execute = NULL;

	c_destroy(&mpcodes);

	dynafun_tab_unregister(__mod_tab(MODULE));
	mpc_fini();
	return 0;
}
#endif

/*
 * keep this alphabetically sorted
 */
const char *mpc_dynafuns[] = {
#if !defined(MPC)
	"act",
	"act_around",
	"act_char",
	"act_clan",
	"act_say",
	"act_yell",
	"affect_char",
	"affect_strip",
	"backstab_char",
	"can_backstab",
	"can_loot",
	"can_see",
	"can_wear",
	"cast",
	"cast_char",
	"cast_obj",
	"char_cha",
	"char_clan",
	"char_con",
	"char_dex",
	"char_ethos",
	"char_fighting",
	"char_gold",
	"char_hit",
	"char_hometown",
	"char_int",
	"char_level",
	"char_luck",
	"char_mana",
	"char_max_hit",
	"char_max_mana",
	"char_max_moves",
	"char_moves",
	"char_name_is",
	"char_position",
	"char_quest_time",
	"char_race",
	"char_room",
	"char_sex",
	"char_silver",
	"char_size",
	"char_str",
	"char_vnum",
	"char_wis",
	"close_door",
	"close_obj",
	"create_mob",
	"create_obj",
	"damage",
	"dice",
	"dofun",
	"drop_obj",
	"extract_obj",
	"get_char_area",
	"get_char_here",
	"get_char_room",
	"get_char_world",
	"get_eq_char",
	"get_obj",
	"get_obj_carry",
	"get_obj_here",
	"get_obj_obj",
	"get_obj_room",
	"get_obj_wear",
	"get_obj_world",
	"get_pet",
	"get_pulse",
	"get_random_fighting",
	"get_room_index",
	"get_skill",
	"give_obj",
	"handle_death",
	"has_detect",
	"has_invis",
	"has_sp",
	"inflict_effect",
	"is_act",
	"is_affected",
	"is_awake",
	"is_class",
	"is_evil",
	"is_ghost",
	"is_good",
	"is_immortal",
	"is_mount",
	"is_neutral",
	"is_npc",
	"is_owner",
	"is_owner_name",
	"is_pumped",
	"is_safe",
	"is_safe_nomessage",
	"is_same_group",
	"is_sn_affected",
	"is_wanted",
	"load_mob",
	"load_obj",
	"lock_door",
	"lock_obj",
	"look_char",
	"mob_interpret",
	"multi_hit",
	"number_bits",
	"number_percent",
	"number_range",
	"obj_carried_by",
	"obj_cast_spell",
	"obj_cost",
	"obj_in",
	"obj_item_type",
	"obj_level",
	"obj_room",
	"obj_timer",
	"obj_to_char",
	"obj_to_obj",
	"obj_to_room",
	"obj_vnum",
	"obj_wear_loc",
	"one_hit",
	"open_door",
	"open_obj",
	"raw_kill",
	"real_char_level",
	"room_is",
	"room_sector",
	"room_vnum",
	"say_spell",
	"set_char_gold",
	"set_char_hit",
	"set_char_mana",
	"set_char_moves",
	"set_char_silver",
	"set_obj_level",
	"set_obj_owner",
	"set_obj_timer",
	"set_weapon_dice_type",
	"social_char",
	"spclass_count",
	"spellfun_call",
	"tell_char",
	"time_hour",
	"transfer_char",
	"transfer_group",
	"umax",
	"umin",
	"unlock_door",
	"unlock_obj",
	"wait_state",
	"weapon_is",
#else
	"nonexistent",
	"number_range",
	"print",
	"print2",
	"prints",
#endif
	NULL
};

int
mpc_init(void)
{
	int_const_t *ic;
	const char **pp;
	module_t m;

	c_init(&glob_syms, &c_info_syms);

	/*
	 * add consts to global symbol table
	 */
	for (ic = ic_tab; ic->name != NULL; ic++) {
		sym_t *s;

		if ((s = c_insert(&glob_syms, ic->name)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (const)",
			    ic->name);
			continue;
		}

		s->name = str_dup(ic->name);
		s->type = SYM_VAR;
		s->s.var.type_tag = ic->type_tag;
		s->s.var.data.i = ic->value;
		s->s.var.is_const = TRUE;
	}

	/*
	 * add dynafuns to global symbol table
	 */
	for (pp = mpc_dynafuns; *pp != NULL; pp++) {
		sym_t *s;

		if ((s = c_insert(&glob_syms, *pp)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (func)", *pp);
			continue;
		}

		s->name = str_dup(*pp);
		s->type = SYM_FUNC;
	}

#if defined(MPC)
	init_dynafuns();
#endif

	m.dlh = dlopen(NULL, RTLD_NOW);
	if (m.dlh == NULL) {
#if defined(MPC)
		fprintf(stderr, "dlopen: %s", dlerror());
		exit(1);
#else
		log(LOG_ERROR, "%s: dlopen: %s", __FUNCTION__, dlerror());
		return -1;
#endif
	}

	dynafun_tab_register(core_dynafun_tab, &m);
	return 0;
}

void
mpc_fini()
{
	dynafun_tab_unregister(core_dynafun_tab);
}
