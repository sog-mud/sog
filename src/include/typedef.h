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
 * $Id: typedef.h,v 1.24 1999-06-10 18:19:02 fjoe Exp $
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
typedef struct pc_data 			PC_DATA;
typedef struct reset_data		RESET_DATA;
typedef struct room_index_data 		ROOM_INDEX_DATA;
typedef struct shop_data		SHOP_DATA;
typedef struct room_affect_data		ROOM_AFFECT_DATA;
typedef struct time_info_data		TIME_INFO_DATA;
typedef struct weather_data		WEATHER_DATA;
typedef struct room_history_data	ROOM_HISTORY_DATA;
typedef struct mptrig			MPTRIG;
typedef struct mpcode			MPCODE;
typedef struct qtrouble_t		qtrouble_t;
typedef struct flag_t			flag_t; 

typedef struct class_t			class_t;
typedef struct race_t			race_t;
typedef struct clan_t			clan_t;
typedef struct pcrace_t			pcrace_t;
typedef struct rclass_t			rclass_t;

typedef struct skill_t			skill_t;
typedef struct cskill_t			cskill_t;
typedef struct rskill_t			rskill_t;
typedef struct clskill_t		clskill_t;
typedef struct pcskill_t		pcskill_t;

typedef struct where_t			where_t;
typedef struct namedp_t			namedp_t;
typedef struct lang_t			lang_t;
typedef struct cmd_t			cmd_t;
typedef struct pose_t			pose_t;
typedef struct rulecl_t			rulecl_t;
typedef struct olced_t 			olced_t;	
typedef struct rule_t			rule_t;
typedef union vo_t			vo_t;
typedef struct altar_t			altar_t;
typedef struct hometown_t		hometown_t;
typedef struct note_t			note_t;
typedef struct ban_t			ban_t;

union vo_t {
	int			vnum;
	ROOM_INDEX_DATA *	r;
	OBJ_INDEX_DATA *	o;
	MOB_INDEX_DATA *	m;
};

typedef void	DO_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool	SPEC_FUN	(CHAR_DATA *ch);
typedef void	SPELL_FUN	(int sn, int level, CHAR_DATA *ch, void *vo,
				 int target);
typedef int	OPROG_FUN	(OBJ_DATA *obj, CHAR_DATA *ch, const void *arg);
typedef void	EVENT_FUN	(ROOM_INDEX_DATA *room, CHAR_DATA *ch,
				 ROOM_AFFECT_DATA *raf);
#define args(a) a
#define DECLARE_DO_FUN(fun)	DO_FUN	  fun
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun)	SPELL_FUN fun
#define DECLARE_OPROG_FUN(fun)	OPROG_FUN fun
#define DECLARE_EVENT_FUN(fun)	EVENT_FUN fun

#define DO_FUN(fun)	void	fun(CHAR_DATA *ch, const char *argument)
#define SPEC_FUN(fun)	bool	fun(CHAR_DATA *ch)
#define SPELL_FUN(fun)	void	fun(int sn, int level, CHAR_DATA *ch, void *vo, int target)
#define OPROG_FUN(fun)	int	fun(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);
#define EVENT_FUN(fun)	void	fun(ROOM_INDEX_DATA *room, CHAR_DATA *ch, ROOM_AFFECT_DATA *raf)

/* WIN32 Microsoft specific definitions */
#if defined (WIN32)
#	define vsnprintf	_vsnprintf
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#	define inline
#endif
 
/* 64-bit int value is compiler-specific (not a ANSI standard) */
#if defined (WIN32)
typedef __int64		flag64_t;	/* For MSVC4.2/5.0 - flags */
typedef __int32		flag32_t;	/* short flags (less memory usage) */
typedef unsigned int	u_int;
typedef unsigned char	u_char;
#else
typedef int64_t		flag64_t;	/* For GNU C compilers - flags */
typedef int32_t		flag32_t;	/* short flags (less memory usage) */
#endif
  
#define IS_NULLSTR(str)		(!(str) || *(char*)(str) == '\0')

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))

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
