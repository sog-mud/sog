/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: typedef.h,v 1.77 2004-06-28 19:20:58 tatyana Exp $
 */

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <sys/types.h>
#if defined(LINUX)
#include <stdint.h>
#endif
#include <stdarg.h>

#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

/*
 * mpc and dynafun code relies on this to be the same width as int
 */
#ifndef __cplusplus
typedef int bool;
#endif
typedef int flag_t;

#if defined (WIN32)
typedef unsigned int	u_int;
typedef unsigned char	u_char;
#endif

typedef struct affect_data		AFFECT_DATA;
typedef struct area_data		AREA_DATA;
typedef struct buf_data			BUFFER;
typedef struct char_data		CHAR_DATA;
typedef struct descriptor_data		DESCRIPTOR_DATA;
typedef struct exit_data		EXIT_DATA;
typedef struct ed_data			ED_DATA;
typedef struct help_data		HELP_DATA;
typedef struct mob_index_data		MOB_INDEX_DATA;
typedef struct obj_data			OBJ_DATA;
typedef struct obj_index_data		OBJ_INDEX_DATA;
typedef struct npc_data			NPC_DATA;
typedef struct pc_data			PC_DATA;
typedef struct reset_data		RESET_DATA;
typedef struct room_index_data		ROOM_INDEX_DATA;
typedef struct shop_data		SHOP_DATA;
typedef struct time_info_data		TIME_INFO_DATA;
typedef struct weather_data		WEATHER_DATA;
typedef struct room_history_data	ROOM_HISTORY_DATA;
typedef struct auction_data		AUCTION_DATA;
typedef struct qtrouble_t		qtrouble_t;
typedef struct flaginfo_t		flaginfo_t;
typedef struct actopt_t			actopt_t;

typedef struct class_t			class_t;
typedef struct race_t			race_t;
typedef struct clan_t			clan_t;
typedef struct pcrace_t			pcrace_t;
typedef struct rclass_t			rclass_t;
typedef struct form_index_t		form_index_t;
typedef struct form_t			form_t;

typedef struct lang_t			lang_t;
typedef struct cmd_t			cmd_t;
typedef struct pose_t			pose_t;
typedef struct rulecl_t			rulecl_t;
typedef struct olc_cmd_t		olc_cmd_t;
typedef struct olced_t			olced_t;
typedef struct rule_t			rule_t;
typedef struct module_t			module_t;
typedef union vo_t			vo_t;
typedef struct altar_t			altar_t;
typedef struct hometown_t		hometown_t;
typedef struct note_t			note_t;
typedef struct rating_t			rating_t;
typedef struct ban_t			ban_t;
typedef struct trig_t			trig_t;
typedef struct mprog_t			mprog_t;
typedef struct spec_t			spec_t;
typedef struct c_ops_t			c_ops_t;
typedef struct avltree_info_t		avltree_info_t;
typedef struct avlnode_t		avlnode_t;
typedef struct avltree_t		avltree_t;
typedef struct container_t		container_t;
typedef struct vo_iter_class_t		vo_iter_class_t;
typedef struct vo_iter_t		vo_iter_t;
typedef struct mpc_iter_t		mpc_iter_t;
typedef struct hint_t			hint_t;
typedef struct var_t			var_t;
typedef struct pc_skill_t		pc_skill_t;
typedef struct effect_t			effect_t;
typedef struct uhandler_t		uhandler_t;

#if !defined(NO_MMAP)
typedef struct rfile_t			rfile_t;
#else
typedef FILE				rfile_t;
#endif

#define UNUSED_ARG(a)			((void) &a)
#define CAST(T, p)			((T) (uintptr_t) (const void *) (p))

union vo_t {
	int vnum;
	int i;
	const char *s;
	ROOM_INDEX_DATA *r;
	OBJ_INDEX_DATA *o;
	MOB_INDEX_DATA *m;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	AFFECT_DATA *aff;
	void *p;
};

#define INT(v)		((v).i)
#define STR(v)		(IS_NULLSTR((v).s) ? str_empty: (v).s)
#define STR1(v)		(IS_NULLSTR((v).s) ? "none" : (v).s)
#define FLAGS(v)	(format_flags(INT(v)))
#define SFLAGS(t, v)	(flag_string((t), INT(v)))
#define STR_ASSIGN(v, str)			\
	do {					\
		free_string((v).s);		\
		(v).s = (str);			\
	} while(0);

typedef void	DO_FUN		(CHAR_DATA *, const char *);
typedef void	EVENT_FUN	(CHAR_DATA *, AFFECT_DATA *);
typedef void	SPELL_FUN	(const char *, int, CHAR_DATA *, void *);
typedef void	UPDATE_FUN	(void);
typedef bool	UPDATE_FOREACH_FUN(void *);
typedef int	MODINIT_FUN	(module_t *);
typedef void	RUNGAME_FUN	(fd_set *, fd_set *, fd_set *);
typedef void	EFFECT_FUN	(void *vo, int level, int dam);

#define DECLARE_DO_FUN(fun)		DO_FUN fun
#define DECLARE_EVENT_FUN(fun)		EVENT_FUN fun
#define DECLARE_SPELL_FUN(fun)		SPELL_FUN fun
#define DECLARE_UPDATE_FUN(fun)		UPDATE_FUN fun
#define DECLARE_UPDATE_FOREACH_FUN(fun)	UPDATE_FOREACH_FUN fun
#define DECLARE_MODINIT_FUN(fun)	MODINIT_FUN fun
#define DECLARE_RUNGAME_FUN(fun)	RUNGAME_FUN fun
#define DECLARE_EFFECT_FUN(fun)		EFFECT_FUN fun

#define DO_FUN(fun, ch, argument)					\
	void fun(CHAR_DATA *ch __attribute__((unused)),			\
		 const char *argument __attribute__((unused)))
#define EVENT_FUN(fun, ch, af)						\
	void fun(CHAR_DATA *ch, AFFECT_DATA *af __attribute__((unused)))
#define SPELL_FUN(fun, sn, level, ch, vo)				\
	void fun(const char *sn __attribute__((unused)),		\
		 int level __attribute__((unused)),			\
		 CHAR_DATA *ch __attribute__((unused)),			\
		 void *vo __attribute__((unused)))
#define	UPDATE_FUN(fun)							\
	void fun(void)
#define	UPDATE_FOREACH_FUN(fun, vo)					\
	bool fun(void *vo)
#define MODINIT_FUN(fun, m)						\
	int fun(module_t *m __attribute__((unused)))
#define RUNGAME_FUN(fun, in_set, out_set, exc_set)			\
	void fun(fd_set *in_set, fd_set *out_set,			\
		 fd_set *exc_set __attribute__((unused)))
#define EFFECT_FUN(fun)							\
	void fun(void *vo, int level, int dam)

typedef void (*e_init_t)(void *);
typedef void (*e_destroy_t)(void *);

typedef int (*ke_cmp_t)(const void *k, const void *e);

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define DIFF_BIT(a, b)		(~(~(a) | (b)))

/* RT ASCII conversions */

#define A	((flag_t) 1 <<  0)
#define B	((flag_t) 1 <<  1)
#define C	((flag_t) 1 <<  2)
#define D	((flag_t) 1 <<  3)
#define E	((flag_t) 1 <<  4)
#define F	((flag_t) 1 <<  5)
#define G	((flag_t) 1 <<  6)
#define H	((flag_t) 1 <<  7)

#define I	((flag_t) 1 <<  8)
#define J	((flag_t) 1 <<  9)
#define K	((flag_t) 1 << 10)
#define L	((flag_t) 1 << 11)
#define M	((flag_t) 1 << 12)
#define N	((flag_t) 1 << 13)
#define O	((flag_t) 1 << 14)
#define P	((flag_t) 1 << 15)

#define Q	((flag_t) 1 << 16)
#define R	((flag_t) 1 << 17)
#define S	((flag_t) 1 << 18)
#define T	((flag_t) 1 << 19)
#define U	((flag_t) 1 << 20)
#define V	((flag_t) 1 << 21)
#define W	((flag_t) 1 << 22)
#define X	((flag_t) 1 << 23)

#define Y	((flag_t) 1 << 24)
#define Z	((flag_t) 1 << 25)
#define aa	((flag_t) 1 << 26) /* letters doubled due to conflicts */
#define bb	((flag_t) 1 << 27)
#define cc	((flag_t) 1 << 28)
#define dd	((flag_t) 1 << 29)
#define ee	((flag_t) 1 << 30)

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

#define ISLOWER(c)		(islower((u_char) (c)))
#define ISUPPER(c)		(isupper((u_char) (c)))
#define LOWER(c)		(tolower((u_char) (c)))
#define UPPER(c)		(toupper((u_char) (c)))

#endif
