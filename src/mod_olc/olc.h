/*
 * $Id: olc.h,v 1.47 1999-10-17 08:55:44 fjoe Exp $
 */

/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */

#ifndef _OLC_H_
#define _OLC_H_

/* some standard headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

typedef	bool OLC_FUN(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd);
typedef bool VALIDATE_FUN(CHAR_DATA *ch, const void *arg);

#define DECLARE_OLC_FUN(fun)		OLC_FUN		fun
#define DECLARE_VALIDATE_FUN(fun)	VALIDATE_FUN	fun

#define OLC_FUN(fun)		bool fun(CHAR_DATA *ch, const char *argument, \
					 olc_cmd_t *cmd)
#define VALIDATE_FUN(fun)	bool fun(CHAR_DATA *ch, const void *arg)

/* functions all cmd tables must have */
enum {
	FUN_CREATE,
	FUN_EDIT,
	FUN_SAVE,

	FUN_TOUCH,
	FUN_FIRST = FUN_TOUCH,
	FUN_SHOW,
	FUN_LIST,

	FUN_MAX
};

/*
 * changed flags
 */
#define CF_MSGDB	(A)
#define CF_SOCIAL	(B)
#define CF_CMD		(C)
#define CF_SKILL	(D)

extern int changed_flags;

extern const char ED_AREA	[];
extern const char ED_ROOM	[];
extern const char ED_OBJ	[];
extern const char ED_MOB	[];
extern const char ED_MPCODE	[];
extern const char ED_HELP	[];
extern const char ED_CLAN	[];
extern const char ED_MSG	[];
extern const char ED_CLASS	[];
extern const char ED_LANG	[];
extern const char ED_IMPL	[];
extern const char ED_EXPL	[];
extern const char ED_SOCIAL	[];
extern const char ED_CMD	[];
extern const char ED_SKILL	[];
extern const char ED_RACE	[];
extern const char ED_CLASS	[];

struct olc_cmd_t
{
	char *		name;
	OLC_FUN *	olc_fun;
	VALIDATE_FUN *	validator;
	void*		arg1;
};

/*
 * Interpreter Table Prototypes
 */
extern olc_cmd_t	olc_cmds_area[];
extern olc_cmd_t	olc_cmds_room[];
extern olc_cmd_t	olc_cmds_obj[];
extern olc_cmd_t	olc_cmds_mob[];
extern olc_cmd_t	olc_cmds_mpcode[];
extern olc_cmd_t	olc_cmds_help[];
extern olc_cmd_t	olc_cmds_clan[];
extern olc_cmd_t	olc_cmds_msg[];
extern olc_cmd_t	olc_cmds_class[];
extern olc_cmd_t	olc_cmds_lang[];
extern olc_cmd_t	olc_cmds_impl[];
extern olc_cmd_t	olc_cmds_expl[];
extern olc_cmd_t	olc_cmds_soc[];
extern olc_cmd_t	olc_cmds_cmd[];
extern olc_cmd_t	olc_cmds_skill[];
extern olc_cmd_t	olc_cmds_race[];
extern olc_cmd_t	olc_cmds_class[];

bool olced_obj_busy(CHAR_DATA *ch);
bool olced_busy(CHAR_DATA *ch, const char *id, void *edit, void *edit2);

/*
 * Generic data edit functions
 */
DECLARE_OLC_FUN(olced_spell_out);

bool olced_number	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, int*);
bool olced_name		(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, const char **pStr);
bool olced_str		(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, const char **pStr);
bool olced_str_text	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, const char **pStr);
bool olced_mlstr	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, mlstring*);
bool olced_mlstrnl	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, mlstring*);
bool olced_mlstr_text	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, mlstring*);
bool olced_exd		(CHAR_DATA *ch, const char* argument,
			 olc_cmd_t *cmd, ED_DATA**);
bool olced_flag64	(CHAR_DATA *ch, const char* argument,
			 olc_cmd_t *cmd, flag64_t*);
bool olced_flag32	(CHAR_DATA *ch, const char* argument,
			 olc_cmd_t *cmd, flag32_t*);
bool olced_dice		(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, int *dice);
bool olced_clan		(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, int *clan);
bool olced_rulecl	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, lang_t *l);
bool olced_vform_add	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, rule_t *r);
bool olced_vform_del	(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, rule_t *r);
bool olced_ival		(CHAR_DATA *ch, const char *argument,
			 olc_cmd_t *cmd, int *);

DECLARE_VALIDATE_FUN(validate_filename);
DECLARE_VALIDATE_FUN(validate_room_vnum);
DECLARE_VALIDATE_FUN(validate_skill_spec);

DECLARE_OLC_FUN(show_commands	);
DECLARE_OLC_FUN(show_version	);

AREA_DATA *get_edited_area(CHAR_DATA *ch);

bool	touch_area	(AREA_DATA*);
bool	touch_vnum	(int vnum);
bool	touch_clan	(clan_t *clan);
bool	touch_race	(race_t *clan);
bool	touch_class	(class_t *clan);

void		edit_done	(DESCRIPTOR_DATA *d);
olced_t *	olced_lookup	(const char * id);

bool		olc_trusted	(CHAR_DATA *ch, int min_sec);
FILE *		olc_fopen	(const char *path, const char *file,
				 CHAR_DATA *ch, int min_sec);
void		olc_printf	(CHAR_DATA *ch, const char *format, ...);

#define	SECURITY_CLAN		5
#define	SECURITY_CLAN_PLIST	9
#define SECURITY_SOCIALS	5
#define SECURITY_HELP		5
#define SECURITY_AREA_CREATE	9
#define SECURITY_MSGDB		3
#define SECURITY_RACE		4
#define SECURITY_CLASS		9

#endif

