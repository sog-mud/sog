/*
 * $Id: olc.h,v 1.25 1998-12-23 16:11:20 fjoe Exp $
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

DECLARE_DO_FUN(do_help		);

bool		run_olc_editor	(DESCRIPTOR_DATA *d);
const char	*olc_ed_name	(CHAR_DATA *ch);

typedef	bool OLC_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool VALIDATE_FUN	(CHAR_DATA *ch, const void *arg);

#define DECLARE_OLC_FUN(fun)		OLC_FUN		fun
#define DECLARE_VALIDATE_FUN(fun)	VALIDATE_FUN	fun

#define OLC_FUN(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define VALIDATE_FUN(fun)	bool fun(CHAR_DATA *ch, const void *arg)

/* functions all cmd tables must have */
enum {
	FUN_CREATE,
	FUN_EDIT,
	FUN_TOUCH,

	FUN_FIRST = FUN_TOUCH,

	FUN_SHOW,
	FUN_LIST,

	FUN_MAX
};

/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_data
{
	char *		name;
	OLC_FUN *	olc_fun;
	void*		arg1;
};
typedef struct olc_cmd_data OLC_CMD_DATA;

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
extern const char ED_GENDER	[];
extern const char ED_CASE	[];

/*
 * Interpreter Table Prototypes
 */
extern OLC_CMD_DATA	olc_cmds_area[];
extern OLC_CMD_DATA	olc_cmds_room[];
extern OLC_CMD_DATA	olc_cmds_obj[];
extern OLC_CMD_DATA	olc_cmds_mob[];
extern OLC_CMD_DATA	olc_cmds_mpcode[];
extern OLC_CMD_DATA	olc_cmds_help[];
extern OLC_CMD_DATA	olc_cmds_clan[];
extern OLC_CMD_DATA	olc_cmds_msg[];
extern OLC_CMD_DATA	olc_cmds_class[];
extern OLC_CMD_DATA	olc_cmds_lang[];
extern OLC_CMD_DATA	olc_cmds_word[];

/*
 * Editor Commands.
 */
DECLARE_DO_FUN(do_alist		);
DECLARE_DO_FUN(do_asave		);
DECLARE_DO_FUN(do_ashow		);
DECLARE_DO_FUN(do_edit		);
DECLARE_DO_FUN(do_create	);
DECLARE_DO_FUN(do_resets	);

/*
 * Generic data edit functions
 */
bool olced_number	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, int*);
bool olced_str		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, const char**pStr);
bool olced_str_text	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, const char **pStr);
bool olced_mlstr	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_mlstrnl	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_mlstr_text	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_exd		(CHAR_DATA *ch, const char* argument, ED_DATA**);
bool olced_flag		(CHAR_DATA *ch, const char* argument,
			 OLC_FUN *olc_fun, flag_t*);
bool olced_dice		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, int *dice);
bool olced_clan		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, int *clan);

DECLARE_VALIDATE_FUN(validate_filename);
DECLARE_VALIDATE_FUN(validate_room_vnum);

bool	show_commands	(CHAR_DATA *ch, const char *argument);
bool	show_version	(CHAR_DATA *ch, const char *argument);

AREA_DATA *get_edited_area(CHAR_DATA *ch);

bool	touch_area	(AREA_DATA*);
bool	touch_vnum	(int vnum);

void		edit_done	(DESCRIPTOR_DATA *d);
OLC_CMD_DATA *	olc_cmd_lookup	(CHAR_DATA *ch, OLC_FUN *fun);

#define	SECURITY_CLAN		5
#define SECURITY_HELP		5
#define SECURITY_AREA_CREATE	9
#define SECURITY_MSGDB		3

#endif

