#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/*
 * $Id: typedef.h,v 1.2 1998-09-10 22:07:54 fjoe Exp $
 */

#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

typedef int bool;

#if	!defined(NULL)
#define	NULL	0
#endif

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
typedef struct skill_data		SKILL_DATA;
typedef struct namedp			NAMEDP;

typedef int	OPROG_FUN	(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);
typedef void	DO_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool	SPEC_FUN	(CHAR_DATA *ch);
typedef void	SPELL_FUN	(int sn, int level, CHAR_DATA *ch, void *vo,
				 int target);

#define args(a) a
#define DECLARE_DO_FUN(fun)	DO_FUN	  fun
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun)	SPELL_FUN fun

typedef u_int64_t flag_t;	/* flags */
typedef u_int32_t sflag_t;	/* short flags (less memory usage) */

#define IS_NULLSTR(str)		((str) == NULL || *(char*)(str) == '\0')

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c) 	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define ENTRE(min, num, max)	((min) < (num) && (num) < (max))

#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)

#define ISLOWER(c)		(islower(c))
#define ISUPPER(c)		(isupper(c))
#define LOWER(c)		(tolower(c))
#define UPPER(c)		(toupper(c))

#endif
