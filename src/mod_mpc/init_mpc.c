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
 * $Id: init_mpc.c,v 1.32 2001-09-13 16:22:06 fjoe Exp $
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
#include "mpc_iter.h"

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

static varrdata_t v_swjumps = {
	&varr_ops, NULL, NULL,

	sizeof(swjump_t), 4
};

static void
jumptab_init(varr *v)
{
	c_init(v, &v_swjumps);
}

static void
jumptab_destroy(varr *v)
{
	c_destroy(v);
}

static varrdata_t v_ints = {
	&varr_ops, NULL, NULL,

	sizeof(int), 8
};

static varrdata_t v_jumptabs = {
	&varr_ops,

	(e_init_t) jumptab_init,
	(e_destroy_t) jumptab_destroy,

	sizeof(varr), 4
};

static varrdata_t v_iterdata = {
	&varr_ops, NULL, NULL,

	sizeof(iterdata_t), 4
};

static varrdata_t v_vos = {
	&varr_ops, NULL, NULL,

	sizeof(vo_t), 4
};

static avltree_info_t c_info_strings = {
	&avltree_ops,

	strkey_init,
	strkey_destroy,

	MT_PVOID, sizeof(char *), ke_cmp_csstr,
};

static void
mpcode_init(mpcode_t *mpc)
{
	mpc->name = str_empty;
	mpc->mp = NULL;
	mpc->lineno = 0;

	c_init(&mpc->strings, &c_info_strings);
	c_init(&mpc->syms, &c_info_syms);

	c_init(&mpc->cstack, &v_ints);
	c_init(&mpc->args, &v_ints);
	mpc->curr_block = 0;

	mpc->curr_jumptab = -1;
	mpc->curr_break_addr = INVALID_ADDR;
	mpc->curr_continue_addr = INVALID_ADDR;

	mpc->ip = 0;
	c_init(&mpc->code, &v_ints);

	c_init(&mpc->jumptabs, &v_jumptabs);
	c_init(&mpc->iterdata, &v_iterdata);

	c_init(&mpc->data, &v_vos);
}

static void
mpcode_destroy(mpcode_t *mpc)
{
	free_string(mpc->name);

	c_destroy(&mpc->strings);
	c_destroy(&mpc->syms);

	c_destroy(&mpc->cstack);
	c_destroy(&mpc->args);

	c_destroy(&mpc->code);

	c_destroy(&mpc->jumptabs);
	c_destroy(&mpc->iterdata);

	c_destroy(&mpc->data);
}

avltree_info_t c_info_mpcodes = {
	&avltree_ops,

	(e_init_t) mpcode_init,
	(e_destroy_t) mpcode_destroy,

	MT_PVOID, sizeof(mpcode_t), ke_cmp_csstr
};

static
FOREACH_CB_FUN(compile_mprog_cb, p, ap)
{
	mprog_t *mp = (mprog_t *) p;

	if (mprog_compile(mp) < 0) {
		log(LOG_INFO, "load_mprog: %s (%s)",
		    mp->name, flag_string(mprog_types, mp->type));
		fprintf(stderr, "%s", buf_string(mp->errbuf));
	}

	return NULL;
}

MODINIT_FUN(_module_load, m)
{
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

	if (iter_init(m) < 0)
		return -1;

	if (mpc_init() < 0)
		return -1;

	dynafun_tab_register(__mod_tab(MODULE), m);

	c_init(&mpcodes, &c_info_mpcodes);
	c_foreach(&mprogs, compile_mprog_cb);

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
	"acid_effect",
	"act",
	"act_char",
	"act_say",
	"act_yell",
	"affect_char",
	"affect_strip",
	"can_see",
	"cast",
	"cast_char",
	"cast_obj",
	"char_clan",
	"char_fighting",
	"char_gold",
	"char_hit",
	"char_level",
	"char_max_hit",
	"char_position",
	"char_race",
	"char_room",
	"char_sex",
	"char_silver",
	"char_size",
	"close_door",
	"close_obj",
	"cold_effect",
	"create_obj",
	"damage",
	"dice",
	"dofun",
	"fire_effect",
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
	"get_random_fighting",
	"get_pulse",
	"get_room_index",
	"give_obj",
	"handle_death",
	"has_sp",
	"is_affected",
	"is_awake",
	"is_class",
	"is_evil",
	"is_ghost",
	"is_good",
	"is_immortal",
	"is_neutral",
	"is_npc",
	"is_owner",
	"is_sn_affected",
	"is_wanted",
	"load_obj",
	"lock_door",
	"lock_obj",
	"look_char",
	"mob_interpret",
	"multi_hit",
	"number_bits",
	"number_percent",
	"number_range",
	"obj_cast_spell",
	"obj_item_type",
	"obj_level",
	"obj_timer",
	"obj_to_char",
	"obj_to_obj",
	"obj_to_room",
	"obj_wear_loc",
	"obj_vnum",
	"one_hit",
	"open_door",
	"open_obj",
	"poison_effect",
	"purge_obj",
	"real_char_level",
	"room_sector",
	"sand_effect",
	"scream_effect",
	"set_char_gold",
	"set_char_hit",
	"set_char_silver",
	"set_obj_level",
	"set_obj_timer",
	"set_weapon_dice_type",
	"shock_effect",
	"social_char",
	"spclass_count",
	"spellfun_call",
	"transfer_char",
	"transfer_group",
	"tell_char",
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

	m.dlh = dlopen(NULL, 0);
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
