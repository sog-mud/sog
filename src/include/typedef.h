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
 * $Id: typedef.h,v 1.39 1999-12-21 06:36:26 fjoe Exp $
 */

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <sys/types.h>

#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

typedef int bool;

#if defined (WIN32)
typedef unsigned int			uint;
#endif

typedef struct affect_data		AFFECT_DATA;
typedef struct area_data		AREA_DATA;
typedef struct buf_data			BUFFER;
typedef struct char_data		CHAR_DATA;
typedef struct descriptor_data	 	DESCRIPTOR_DATA;
typedef struct exit_data		EXIT_DATA;
typedef struct ed_data			ED_DATA;
typedef struct help_data		HELP_DATA;
typedef struct mob_index_data		MOB_INDEX_DATA;
typedef struct obj_data			OBJ_DATA;
typedef struct obj_index_data		OBJ_INDEX_DATA;
typedef struct npc_data			NPC_DATA;
typedef struct pc_data 			PC_DATA;
typedef struct reset_data		RESET_DATA;
typedef struct room_index_data 		ROOM_INDEX_DATA;
typedef struct shop_data		SHOP_DATA;
typedef struct time_info_data		TIME_INFO_DATA;
typedef struct weather_data		WEATHER_DATA;
typedef struct room_history_data	ROOM_HISTORY_DATA;
typedef struct mptrig			MPTRIG;
typedef struct mpcode			MPCODE;
typedef struct qtrouble_t		qtrouble_t;
typedef struct flaginfo_t			flaginfo_t; 

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
typedef struct olced_t 			olced_t;	
typedef struct rule_t			rule_t;
typedef struct module_t			module_t;
typedef union vo_t			vo_t;
typedef struct altar_t			altar_t;
typedef struct hometown_t		hometown_t;
typedef struct note_t			note_t;

#ifdef USE_MMAP
typedef struct rfile_t			rfile_t;
#else
typedef FILE				rfile_t;
#endif

union vo_t {
	int vnum;
	int i;
	const char *s;
	ROOM_INDEX_DATA *r;
	OBJ_INDEX_DATA *o;
	MOB_INDEX_DATA *m;
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

typedef void	DO_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool	SPEC_FUN	(CHAR_DATA *ch);
typedef void	SPELL_FUN	(const char *sn, int level,
				 CHAR_DATA *ch, void *vo);
typedef int	OPROG_FUN	(OBJ_DATA *obj, CHAR_DATA *ch, const void *arg);
typedef void	EVENT_FUN	(CHAR_DATA *ch, AFFECT_DATA *af);

#define args(a) a
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_DO_FUN(fun) 	DO_FUN fun
#define DECLARE_OPROG_FUN(fun)	OPROG_FUN fun
#define DECLARE_EVENT_FUN(fun)	EVENT_FUN fun

#define SPEC_FUN(fun)	bool	fun(CHAR_DATA *ch)
#define OPROG_FUN(fun)	int	fun(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);
#define EVENT_FUN(fun)	void	fun(CHAR_DATA *ch, AFFECT_DATA *af)

/* WIN32 Microsoft specific definitions */
#if defined (WIN32)
#	define vsnprintf	_vsnprintf
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#	define inline
#endif
 
/* 64-bit int value is compiler-specific (not a ANSI standard) */
#if defined (WIN32)
typedef __int32		flag_t;		/* flags (less memory usage) */
typedef unsigned int	u_int;
typedef unsigned char	u_char;
#else
typedef int32_t		flag_t;		/* flags (less memory usage) */
#endif
  
#define IS_NULLSTR(str)		(!(str) || *(char*)(str) == '\0')

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define DIFF_BIT(a, b)		(~(~(a) | (b)))

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c) 	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define ENTRE(min, num, max)	((min) < (num) && (num) < (max))

#define IS_VALID(data)		((data))
#define VALIDATE(data)
#define INVALIDATE(data)

#define ISLOWER(c)		(islower((unsigned char) (c)))
#define ISUPPER(c)		(isupper((unsigned char) (c)))
#define LOWER(c)		(tolower((unsigned char) (c)))
#define UPPER(c)		(toupper((unsigned char) (c)))

#endif
