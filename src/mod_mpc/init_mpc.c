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
 * $Id: init_mpc.c,v 1.58 2004-02-13 14:48:14 fjoe Exp $
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

mpcode_t *current_mpc;

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

	sizeof(vo_iter_t), 4
};

static varr_info_t c_info_affects = {
	&varr_ops, NULL, NULL,

	sizeof(AFFECT_DATA *), 4
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
	c_init(&mpc->affects, &c_info_affects);

	c_init(&mpc->data, &c_info_vos);
}

static void
mpcode_destroy(mpcode_t *mpc)
{
	AFFECT_DATA **paf;

	free_string(mpc->name);

	c_destroy(&mpc->strings);
	c_destroy(&mpc->syms);
	c_destroy(&mpc->svhs);

	c_destroy(&mpc->cstack);
	c_destroy(&mpc->args);

	c_destroy(&mpc->code);

	c_destroy(&mpc->jumptabs);
	c_destroy(&mpc->iters);
	C_FOREACH(paf, &mpc->affects)
		aff_free(*paf);
	c_destroy(&mpc->affects);

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
		printlog(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	mprog_execute = dlsym(m->dlh, "_mprog_execute");	// notrans
	if (mprog_execute == NULL) {
		printlog(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	if (mpc_init() < 0)
		return -1;

	dynafun_tab_register(__mod_tab(MODULE), m);

	c_init(&mpcodes, &c_info_mpcodes);
	C_FOREACH(mp, &mprogs) {
		if (mprog_compile(mp) < 0) {
			char *errmsg = buf_string(mp->errbuf);
			int len = strlen(errmsg);

			/* strip trailing slash */
			if (errmsg[len - 1] == '\n')
				len--;
			printlog(LOG_INFO, "load_mprog: %s\n%.*s",
			    format_mpname(mp), len, errmsg);
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
struct mpc_dynafun_t {
	const char *name;
	const char *fun;
};
typedef struct mpc_dynafun_t mpc_dynafun_t;

mpc_dynafun_t mpc_dynafuns[] = {
#if !defined(MPC)
/* core dynafuns */
	{ "dice",		NULL		},
	{ "dofun",		NULL		},
	{ "get_room_index",	NULL		},
	{ "number_bits",	NULL		},
	{ "number_percent",	NULL		},
	{ "number_range",	NULL		},

/* mpc dynafuns */
	{ "aff_new",		"mpc_aff_new"	},
	{ "affect_to_char",	NULL		},
	{ "affect_to_obj",	NULL		},
	{ "affect_to_room",	NULL		},
	{ "can_wear",		NULL		},
	{ "char_form_is",	NULL		},
	{ "char_name_is",	NULL		},
	{ "get_random_fighting",NULL		},
	{ "has_sp",		NULL		},
	{ "has_detect",		NULL		},
	{ "has_invis",		NULL		},
	{ "is_act",		NULL		},
	{ "is_affected",	NULL		},
	{ "is_awake",		NULL		},
	{ "is_evil",		NULL		},
	{ "is_ghost",		NULL		},
	{ "is_good",		NULL		},
	{ "is_immortal",	NULL		},
	{ "is_mount",		NULL		},
	{ "is_neutral",		NULL		},
	{ "is_npc",		NULL		},
	{ "is_owner",		NULL		},
	{ "is_owner_name",	NULL		},
	{ "is_pumped",		NULL		},
	{ "is_wanted",		NULL		},
	{ "load_mob",		NULL		},
	{ "load_obj",		NULL		},
	{ "mob_interpret",	NULL		},
	{ "room_is",		NULL		},
	{ "spclass_count",	NULL		},
	{ "transfer_group",	NULL		},
	{ "time_hour",		NULL		},
	{ "umax",		NULL		},
	{ "umin",		NULL		},
	{ "wait_state",		NULL		},
	{ "weapon_is",		NULL		},

/* exported dynafuns */
	{ "act",		NULL		},
	{ "act_around",		NULL		},
	{ "act_char",		NULL		},
	{ "act_clan",		NULL		},
	{ "act_say",		NULL		},
	{ "act_yell",		NULL		},
	{ "affect_strip",	NULL		},
	{ "affect_strip_obj",	NULL		},
	{ "backstab_char",	NULL		},
	{ "calc_spell_damage",	NULL		},
	{ "can_backstab",	NULL		},
	{ "can_loot",		NULL		},
	{ "can_see",		NULL		},
	{ "cast",		NULL		},
	{ "cast_char",		NULL		},
	{ "cast_obj",		NULL		},
	{ "close_door",		NULL		},
	{ "close_obj",		NULL		},
	{ "create_mob",		NULL		},
	{ "create_obj",		NULL		},
	{ "damage",		NULL		},
	{ "drop_obj",		NULL		},
	{ "extract_obj",	NULL		},
	{ "get_char_area",	NULL		},
	{ "get_char_here",	NULL		},
	{ "get_char_room",	NULL		},
	{ "get_char_world",	NULL		},
	{ "get_eq_char",	NULL		},
	{ "get_obj",		NULL		},
	{ "get_obj_carry",	NULL		},
	{ "get_obj_here",	NULL		},
	{ "get_obj_obj",	NULL		},
	{ "get_obj_room",	NULL		},
	{ "get_obj_wear",	NULL		},
	{ "get_obj_world",	NULL		},
	{ "get_pulse",		NULL		},
	{ "get_skill",		NULL		},
	{ "give_obj",		NULL		},
	{ "handle_death",	NULL		},
	{ "has_spec",		NULL		},
	{ "inflict_effect",	NULL		},
	{ "is_safe",		NULL		},
	{ "is_safe_nomessage",	NULL		},
	{ "is_same_group",	NULL		},
	{ "is_sn_affected",	NULL		},
	{ "is_sn_affected_obj",	NULL		},
	{ "lock_door",		NULL		},
	{ "lock_obj",		NULL		},
	{ "look_char",		NULL		},
	{ "multi_hit",		NULL		},
	{ "obj_cast_spell",	NULL		},
	{ "obj_to_char",	NULL		},
	{ "obj_to_obj",		NULL		},
	{ "obj_to_room",	NULL		},
	{ "one_hit",		NULL		},
	{ "open_door",		NULL		},
	{ "open_obj",		NULL		},
	{ "raw_kill",		NULL		},
	{ "saves_spell",	NULL		},
	{ "say_spell",		NULL		},
	{ "social_char",	NULL		},
	{ "spellfun",		NULL		},
	{ "tell_char",		NULL		},
	{ "transfer_char",	NULL		},
	{ "unlock_door",	NULL		},
	{ "unlock_obj",		NULL		},
#else
	{ "nonexistent",	NULL		},
	{ "number_range",	NULL		},
	{ "print",		NULL		},
	{ "print2",		NULL		},
	{ "prints",		NULL		},
#endif
	{ NULL, NULL }
};

int
mpc_init(void)
{
	int_const_t *ic;
	mpc_dynafun_t *mdf;
	module_t m;

	c_init(&glob_syms, &c_info_syms);

	/*
	 * add consts to global symbol table
	 */
	for (ic = ic_tab; ic->name != NULL; ic++) {
		sym_t *s;

		if ((s = c_insert(&glob_syms, ic->name)) == NULL) {
			printlog(LOG_ERROR, "%s: duplicate symbol (const)",
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
	for (mdf = mpc_dynafuns; mdf->name != NULL; mdf++) {
		sym_t *s;

		if ((s = c_insert(&glob_syms, mdf->name)) == NULL) {
			printlog(LOG_ERROR, "%s: duplicate symbol (func)",
			    mdf->name);
			continue;
		}

		s->name = str_dup(mdf->name);
		s->type = SYM_FUNC;
		if (mdf->fun == NULL)
			s->s.func.name = str_qdup(s->name);
		else
			s->s.func.name = str_dup(mdf->fun);
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
		printlog(LOG_ERROR, "%s: dlopen: %s", __FUNCTION__, dlerror());
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
