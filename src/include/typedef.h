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
 * $Id: typedef.h,v 1.13 1998-10-21 05:00:29 fjoe Exp $
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

typedef struct affect_data		AFFECT_DATA;
typedef struct area_data		AREA_DATA;
typedef struct ban_data			BAN_DATA;
typedef struct buf_data			BUFFER;
typedef struct char_data		CHAR_DATA;
typedef struct descriptor_data	 	DESCRIPTOR_DATA;
typedef struct exit_data		EXIT_DATA;
typedef struct ed_data			ED_DATA;
typedef struct help_data		HELP_DATA;
typedef struct kill_data		KILL_DATA;
typedef struct mob_index_data		MOB_INDEX_DATA;
typedef struct note_data		NOTE_DATA;
typedef struct obj_data			OBJ_DATA;
typedef struct obj_index_data		OBJ_INDEX_DATA;
typedef struct pc_data 			PC_DATA;
typedef struct reset_data		RESET_DATA;
typedef struct room_index_data 		ROOM_INDEX_DATA;
typedef struct shop_data		SHOP_DATA;
typedef struct time_info_data		TIME_INFO_DATA;
typedef struct weather_data		WEATHER_DATA;
typedef struct room_history_data	ROOM_HISTORY_DATA;
typedef struct mptrig			MPTRIG;
typedef struct mpcode			MPCODE;
typedef struct qtrouble_data		QTROUBLE_DATA;
typedef struct mlstring			mlstring;
typedef struct hometown_data		HOMETOWN_DATA;
typedef struct varr			varr;
typedef struct flag			FLAG; 
typedef struct class_skill		CLASS_SKILL;
typedef struct race_skill		RACE_SKILL;
typedef struct clan_skill		CLAN_SKILL;
typedef struct pc_skill			PC_SKILL;
typedef struct clan_data		CLAN_DATA;
typedef struct where_data		WHERE_DATA;
typedef struct class_data		CLASS_DATA;
typedef struct pose_data		POSE_DATA;
typedef struct skill_data		SKILL_DATA;
typedef struct namedp			NAMEDP;
typedef struct lang_data		LANG_DATA;
typedef struct word_data		WORD_DATA;
typedef struct cmd_data			CMD_DATA;

typedef void	DO_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool	SPEC_FUN	(CHAR_DATA *ch);
typedef void	SPELL_FUN	(int sn, int level, CHAR_DATA *ch, void *vo,
				 int target);
typedef int	OPROG_FUN	(OBJ_DATA *obj, CHAR_DATA *ch, const void *arg);

#define args(a) a
#define DECLARE_DO_FUN(fun)	DO_FUN	  fun
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun)	SPELL_FUN fun
#define DECLARE_OPROG_FUN(fun)	OPROG_FUN fun

#define DO_FUN(fun)	void	fun(CHAR_DATA *ch, const char *argument)
#define SPEC_FUN(fun)	bool	fun(CHAR_DATA *ch)
#define SPELL_FUN(fun)	void	fun(int sn, int level, CHAR_DATA *ch, void *vo, int target)
#define OPROG_FUN(fun)	int	fun(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);

typedef int64_t	flag_t;		/* flags */
typedef int32_t	sflag_t;	/* short flags (less memory usage) */

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
