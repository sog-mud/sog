#ifndef _OLC_H_
#define _OLC_H_

/*
 * $Id: olc.h,v 1.13 1998-09-01 18:38:09 fjoe Exp $
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

bool		run_olc_editor	(DESCRIPTOR_DATA *d);
const char	*olc_ed_name	(CHAR_DATA *ch);

typedef	bool OLC_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool VALIDATE_FUN	(CHAR_DATA *ch, const void *arg);

#define DECLARE_OLC_FUN(fun)		OLC_FUN    fun
#define DECLARE_VALIDATE_FUN(fun)	VALIDATE_FUN fun
#define VALIDATOR(fun)			bool fun(CHAR_DATA *ch, const void *arg)

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

/*
 * Connected states for editor.
 */
#define ED_NONE		0
#define ED_AREA		1
#define ED_ROOM		2
#define ED_OBJECT	3
#define ED_MOBILE	4
#define ED_MPCODE	5
#define ED_HELP		6
#define ED_CLAN		7

/*
 * Interpreter Table Prototypes
 */
extern OLC_CMD_DATA	aedit_table[];
extern OLC_CMD_DATA	redit_table[];
extern OLC_CMD_DATA	oedit_table[];
extern OLC_CMD_DATA	medit_table[];
extern OLC_CMD_DATA	mpedit_table[];
extern OLC_CMD_DATA	hedit_table[];
extern OLC_CMD_DATA	cedit_table[];

/*
 * Interpreter Prototypes
 */
void    aedit           (CHAR_DATA *ch, const char *argument);
void    redit           (CHAR_DATA *ch, const char *argument);
void    medit           (CHAR_DATA *ch, const char *argument);
void    oedit           (CHAR_DATA *ch, const char *argument);
void	mpedit		(CHAR_DATA *ch, const char *argument);
void	hedit		(CHAR_DATA *ch, const char *argument);
void	cedit		(CHAR_DATA *ch, const char *argument);

/*
 * Editor Commands.
 */
DECLARE_DO_FUN(do_alist		);
DECLARE_DO_FUN(do_clist		);
DECLARE_DO_FUN(do_asave		);
DECLARE_DO_FUN(do_olc		);
DECLARE_DO_FUN(do_resets	);
DECLARE_DO_FUN(do_aedit		);
DECLARE_DO_FUN(do_redit		);
DECLARE_DO_FUN(do_oedit		);
DECLARE_DO_FUN(do_medit		);
DECLARE_DO_FUN(do_mpedit	);
DECLARE_DO_FUN(do_hedit		);
DECLARE_DO_FUN(do_cedit		);

/*
 * Generic data edit functions
 */
bool olced_number	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, int*);
bool olced_str		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, char**);
bool olced_str_text	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, char **pStr);
bool olced_mlstr	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_mlstrnl	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_mlstr_text	(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *olc_fun, mlstring**);
bool olced_ed		(CHAR_DATA *ch, const char* argument, ED_DATA**);
bool olced_flag		(CHAR_DATA *ch, const char* argument,
			 OLC_FUN *olc_fun, flag_t*);
bool olced_dice		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, int *dice);
bool olced_clan		(CHAR_DATA *ch, const char *argument,
			 OLC_FUN *fun, int *clan);

DECLARE_VALIDATE_FUN(validate_filename);

bool show_commands	(CHAR_DATA *ch, const char *argument);
bool show_version	(CHAR_DATA *ch, const char *argument);
bool edit_done		(CHAR_DATA *ch);

OLC_CMD_DATA *olc_cmd_lookup(CHAR_DATA *ch, OLC_FUN *fun);

#define SECURITY_HELP	9
#define SECURITY_CLAN	5

#endif

