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
 * $Id: olc.c,v 1.169 2004-06-28 19:21:04 tatyana Exp $
 */

/***************************************************************************
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

#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <dlfcn.h>

#include "olc.h"

#include <lang.h>
#include <module.h>
#include <mprog.h>

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

bool _olc_interpret(DESCRIPTOR_DATA *d, const char *argument);

DECLARE_DO_FUN(do_create);
DECLARE_DO_FUN(do_edit);
DECLARE_DO_FUN(do_asave);
DECLARE_DO_FUN(do_alist);
DECLARE_DO_FUN(do_ashow);

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n" \
		"     Port a ROM 2.4 v1.7\n"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n" \
                "     Modified for use with ROM 2.3\n"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n" \
                "     Modificado para uso en ROM 2.4b4a\n"	\
                "     Por Ivan Toledo (pvillanu@choapa.cic.userena.cl)\n" \
		"     Modified for use with SoG\n" \
		"     Farmer Joe (fjoe@iclub.nsu.ru)\n"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n" \
		"     Version actual : 1.71 - Mar 22, 1998\n"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

const char ED_AREA[]	= "areas";
const char ED_ROOM[]	= "rooms";
const char ED_OBJ[]	= "objects";
const char ED_MOB[]	= "mobiles";
const char ED_MPROG[]	= "mprogs";
const char ED_HELP[]	= "helps";
const char ED_CLAN[]	= "clans";
const char ED_MSG[]	= "msgdb";
const char ED_LANG[]	= "languages";
const char ED_IMPL[]	= "implicit";
const char ED_EXPL[]	= "explicit";
const char ED_SOCIAL[]	= "socials";
const char ED_CMD[]	= "commands";
const char ED_SKILL[]	= "skills";
const char ED_SPEC[]	= "specs";
const char ED_RACE[]	= "races";
const char ED_CLASS[]	= "classes";
const char ED_MATERIAL[]= "materials";
const char ED_LIQUID[]	= "liquids";
const char ED_DAMT[]	= "damtypes";
const char ED_HINT[]	= "hints";

olced_t olced_table[] = {
	{ ED_AREA,	"AreaEd",	olc_cmds_area	},
	{ ED_ROOM,	"RoomEd",	olc_cmds_room	},
	{ ED_OBJ,	"ObjEd",	olc_cmds_obj	},
	{ ED_MOB,	"MobEd",	olc_cmds_mob	},
	{ ED_MPROG,	"MProgEd",	olc_cmds_mprog	},
	{ ED_HELP,	"HelpEd",	olc_cmds_help	},
	{ ED_MSG,	"MsgEd",	olc_cmds_msg	},
	{ ED_CLAN,	"ClanEd",	olc_cmds_clan	},
	{ ED_LANG,	"LangEd",	olc_cmds_lang	},
	{ ED_IMPL,	"ImplRuleEd",	olc_cmds_impl	},
	{ ED_EXPL,	"ExplRuleEd",	olc_cmds_expl	},
	{ ED_SOCIAL,	"SocEd",	olc_cmds_soc	},
	{ ED_RACE,	"RaceEd",	olc_cmds_race	},
	{ ED_CLASS,	"ClassEd",	olc_cmds_class	},
	{ ED_MATERIAL,	"MatEd",	olc_cmds_mat	},
	{ ED_LIQUID,	"LiqEd",	olc_cmds_liq	},
	{ ED_SKILL,	"SkillEd",	olc_cmds_skill	},
	{ ED_SPEC,	"SpecEd",	olc_cmds_spec	},
	{ ED_CMD,	"CmdEd",	olc_cmds_cmd	},
	{ ED_DAMT,	"DamtEd",	olc_cmds_damt	},
	{ ED_HINT,	"HintEd",	olc_cmds_hint	},

	{ NULL, NULL, NULL }
};

static olc_cmd_t *	olc_cmd_lookup(olc_cmd_t *cmd_table, const char *name);

static void do_olc(CHAR_DATA *ch, const char *argument, int fun);

static const char *skip_commands[] = { "n", "w", "e", "s", "u", "d" };
#define NSKIP_COMMANDS (sizeof(skip_commands) / sizeof(*skip_commands))

MODINIT_FUN(_module_load, m)
{
	olc_interpret = dlsym(m->dlh, "_olc_interpret");
	if (olc_interpret == NULL) {
		printlog(LOG_INFO, "_module_load(mod_olc): %s", dlerror());
		return -1;
	}

	cmd_mod_load(m);
	qsort(skip_commands, NSKIP_COMMANDS, sizeof(*skip_commands), cmpstr);
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	DESCRIPTOR_DATA *d;

	/* drop all the builders out OLC editors */
	for (d = descriptor_list; d; d = d->next) {
		if (d->connected != CON_PLAYING
		||  d->olced == NULL)
			continue;

		act_char("Unloading OLC module.", d->character);
		if (d->pString)
			string_add_exit(d->character, FALSE);
		edit_done(d);
	}

	cmd_mod_unload(m);
	olc_interpret = NULL;
	return 0;
}

bool
_olc_interpret(DESCRIPTOR_DATA *d, const char *argument)
{
	char command[MAX_INPUT_LENGTH];
	olc_cmd_t *cmd;
	olced_t *olced;
	const char *p = command;

	if ((olced = d->olced) == NULL)
		return FALSE;

	argument = one_argument(argument, command, sizeof(command));

	if (command[0] == '\0') {
		olced->cmd_table[FUN_SHOW].olc_fun(d->character, argument,
						   olced->cmd_table+FUN_SHOW);
		return TRUE;
	}

	if (bsearch(&p, skip_commands, NSKIP_COMMANDS,
		    sizeof(*skip_commands), cmpstr) != NULL)
		return FALSE;

	if (!str_cmp(command, "done")) {
		edit_done(d);
		return TRUE;
	}

	if ((cmd = olc_cmd_lookup(olced->cmd_table+FUN_FIRST, command)) == NULL
	||  cmd->olc_fun == NULL)
		return FALSE;

	if (cmd->olc_fun(d->character, argument, cmd)
	&&  d->character->desc != NULL) {
		olced->cmd_table[FUN_TOUCH].olc_fun(
		    d->character, str_empty, olced->cmd_table+FUN_TOUCH);
	}

	return TRUE;
}

DO_FUN(do_create, ch, argument)
{
	do_olc(ch, argument, FUN_CREATE);
}

DO_FUN(do_edit, ch, argument)
{
	do_olc(ch, argument, FUN_EDIT);
}

DO_FUN(do_asave, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	/*
	 * compatibility with old OLC
	 */
	one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "world"))
		argument = "areas all";
	else if (!str_cmp(arg, "changed"))
		argument = "areas";

	do_olc(ch, argument, FUN_SAVE);
}

DO_FUN(do_alist, ch, argument)
{
	do_olc(ch, argument, FUN_LIST);
}

DO_FUN(do_ashow, ch, argument)
{
	do_olc(ch, argument, FUN_SHOW);
}

/*
 * olced_busy -- returns TRUE if there is another character
 *		 is using the same OLC editor
 */
bool
olced_busy(CHAR_DATA *ch, const char *id, void *edit, void *edit2)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (vch != ch
		&&  d->olced
		&&  d->olced->id == id
		&&  (!edit || d->pEdit == edit)
		&&  (!edit2 || d->pEdit2 == edit2)) {
			act_puts("$t: $T is locking this editor right now.",
				 ch, d->olced->name, vch->name,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Generic OLC editor functions.
 * All functions assume !IS_NPC(ch).
 */
OLC_FUN(olced_spell_out)
{
	act_char("Spell it out.", ch);
	return FALSE;
}

OLC_FUN(olced_strkey)
{
	char arg[MAX_INPUT_LENGTH];
	olced_strkey_t *o;
	const char **pold_key;

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (IS_NULLSTR(arg)) {
		act_puts("Usage: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, arg))
		return FALSE;

	pold_key = (const char **) ch->desc->pEdit;
	o = (olced_strkey_t *) cmd->arg1;

	if (!str_cmp(*pold_key, arg)) {
		/*
		 * nothing to change
		 */
		act_char("Ok.", ch);
		return FALSE;
	}

	if (c_lookup(o->c, arg) != NULL) {
		act_puts("$t: $T: duplicate name.",
			 ch, OLCED(ch)->name, arg,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (o->path) {
		const char *old_file;

		old_file = strkey_filename(*pold_key, o->ext);
		if (dfexist(o->path, old_file)) {
			d2rename(o->path, old_file,
				 o->path, strkey_filename(arg, o->ext));
		}
	}

	c_move(o->c, *pold_key, arg);
	free_string(*pold_key);
	*pold_key = str_dup(arg);

	act_char("Ok.", ch);
	return TRUE;
}

bool
_olced_mlstrkey(CHAR_DATA *ch, const char *langname, const char *argument,
		olc_cmd_t *cmd)
{
	olced_strkey_t *o;
	mlstring *ml;

	lang_t *lang;
	const char **pp;

	if (IS_NULLSTR(argument)) {
		act_puts("Usage: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(langname, "all"))
		lang = NULL;
	else if ((lang = lang_lookup(langname)) == NULL) {
		act_puts("$t: $T: unknown language",
			 ch, OLCED(ch)->name, langname,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	ml = (mlstring *) ch->desc->pEdit;
	o = (olced_strkey_t *) cmd->arg1;

	if (lang == NULL || lang == VARR_GET(&langs, 0)) {
		const char *old_key = mlstr_mval(ml);

		/*
		 * gonna change key
		 */
		if (!str_cmp(old_key, argument)) {
			/*
			 * key remain unchanged
			 */
			act_char("Ok.", ch);
			return TRUE;
		}

		if (c_lookup(o->c, argument) != NULL) {
			act_puts("$t: $T: duplicate name.",
				 ch, OLCED(ch)->name, argument,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		c_move(o->c, old_key, argument);

		if (o->path) {
			const char *old_file;

			old_file = strkey_filename(old_key, o->ext);
			if (dfexist(o->path, old_file)) {
				d2rename(
				    o->path, old_file,
				    o->path, strkey_filename(argument, o->ext));
			}
		}
	}

	/*
	 * make the change
	 */
	pp = mlstr_convert(ml, lang);
	free_string(*pp);
	*pp = str_dup(argument);

	act_char("Ok.", ch);
	return TRUE;
}

OLC_FUN(olced_mlstrkey)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, sizeof(arg));
	return _olced_mlstrkey(ch, arg, argument, cmd);
}

bool
olced_number(CHAR_DATA *ch, const char *argument, olc_cmd_t* cmd, int *pInt)
{
	int val;
	char *endptr;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	val = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0') {
		act_puts("Usage: $t <number>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, &val))
		return FALSE;

	*pInt = val;
	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_name(CHAR_DATA *ch, const char *argument,
	   olc_cmd_t *cmd, const char **pStr)
{
	bool changed;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_puts("Usage: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	changed = FALSE;
	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if (!str_cmp(arg, "all")) {
			act_puts("$t: $T: Illegal name.",
				 ch, OLCED(ch)->name, arg,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			continue;
		}

		/*
		 * call validator only if we are adding a name
		 */
		if (!is_name_strict(arg, *pStr)
		&&  cmd->validator && !cmd->validator(ch, arg))
			continue;

		changed = TRUE;
		name_toggle(pStr, arg, ch, OLCED(ch)->name);
	}

	return changed;
}

bool
olced_foreign_strkey(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, const char **pStr)
{
	void *c;
	void *p;

	if (IS_NULLSTR(argument)) {
		act_puts("Usage: $t <name>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Use '$t ?' for list of valid $Ts.",
			 ch, cmd->name, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Use '$t none' to reset $T.",
			 ch, cmd->name, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		free_string(*pStr);
		*pStr = str_empty;
		return TRUE;
	}

	c = cmd->arg1;
	if (!str_cmp(argument, "?")) {
		BUFFER *out = buf_new(0);
		buf_printf(out, BUF_END, "Valid %ss are:\n", cmd->name);
		c_strkey_dump(c, out);
		page_to_char(buf_string(out), ch);
		buf_free(out);
		return FALSE;
	}

	if ((p = c_strkey_search(c, argument)) == NULL) {
		act_puts("'$t': unknown $T.",
			 ch, argument, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(*pStr);
	*pStr = str_qdup(*(const char**) p);
	return TRUE;
}

bool
olced_foreign_mlstrkey(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, const char **pStr)
{
	void *c;
	void *p;

	if (IS_NULLSTR(argument)) {
		act_puts("Usage: $t <name>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Use '$t ?' for list of valid $Ts.",
			 ch, cmd->name, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Use '$t none' to reset $T.",
			 ch, cmd->name, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		free_string(*pStr);
		*pStr = str_empty;
		return TRUE;
	}

	c = cmd->arg1;
	if (!str_cmp(argument, "?")) {
		BUFFER *out = buf_new(0);
		buf_printf(out, BUF_END, "Valid %ss are:\n", cmd->name);
		c_mlstrkey_dump(c, out);
		page_to_char(buf_string(out), ch);
		buf_free(out);
		return FALSE;
	}

	if ((p = c_mlstrkey_search(c, argument)) == NULL) {
		act_puts("'$t': unknown $T.",
			 ch, argument, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(*pStr);
	*pStr = str_qdup(*(const char**) p);
	return TRUE;
}

bool
olced_str(CHAR_DATA *ch, const char *argument,
	  olc_cmd_t *cmd, const char **pStr)
{
	if (IS_NULLSTR(argument)) {
		act_puts("Usage: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	free_string(*pStr);
	*pStr = str_dup(argument);
	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_str_text(CHAR_DATA *ch, const char *argument,
	       olc_cmd_t *cmd, const char **pStr)
{
	if (argument[0] =='\0') {
		string_append(ch, pStr);
		return FALSE;
	}

	act_puts("Usage: $t",
		 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

bool
olced_mlstr(CHAR_DATA *ch, const char *argument,
	    olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_edit(mlp, argument)) {
		act_puts("Usage: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Usage: $t reset",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_mlstrnl(CHAR_DATA *ch, const char *argument,
	      olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_editnl(mlp, argument)) {
		act_puts("Usage: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Usage: $t reset",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_mlstr_text(CHAR_DATA *ch, const char *argument,
		 olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_append(ch, mlp, argument)) {
		act_puts("Usage: $t <lang>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Usage: $t reset",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	return FALSE;
}

bool
olced_exd(CHAR_DATA *ch, const char* argument,
	  olc_cmd_t *cmd __attribute__((unused)), ED_DATA **ped)
{
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, keyword, sizeof(keyword));

	if (command[0] == '\0' || keyword[0] == '\0')
		OLC_ERROR("'OLC EXD'");

	if (!str_cmp(command, "add")) {
		if (ed_lookup(keyword, *ped) != NULL) {
			act_puts("$t: $T: exd keyword already exists.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		ed		= ed_new();
		ed->keyword	= str_dup(keyword);

		if (!mlstr_append(ch, &ed->description, argument)) {
			ed_free(ed);
			OLC_ERROR("'OLC EXD'");
		}

		ed->next	= *ped;
		*ped		= ed;
		act_char("Extra description added.", ch);
		return TRUE;
	}

	if (!str_cmp(command, "name")) {
		bool changed;
		char arg[MAX_INPUT_LENGTH];

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		changed = FALSE;
		while (TRUE) {
			argument = one_argument(argument, arg, sizeof(arg));
			if (arg[0] == '\0')
				break;

			if (!str_cmp(arg, "none")
			||  !str_cmp(arg, "all")) {
				act_puts("$t: $T: Illegal keyword.",
					 ch, OLCED(ch)->name, arg,
					 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
				continue;
			}

			changed = TRUE;
			name_toggle(&ed->keyword, arg, ch, OLCED(ch)->name);
		}

		return changed;
	}

	if (!str_cmp(command, "edit")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		if (!mlstr_append(ch, &ed->description, argument))
			OLC_ERROR("'OLC EXD'");

		return TRUE;
	}

	if (!str_cmp(command, "delete")) {
		ED_DATA *prev = NULL;
		int num;
		char arg[MAX_INPUT_LENGTH];

		num = number_argument(keyword, arg, sizeof(arg));
		for (ed = *ped; ed != NULL; prev = ed, ed = ed->next) {
			if (arg[0] && !is_name(arg, ed->keyword))
				continue;
			if (!--num)
				break;
		}

		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		if (prev == NULL)
			*ped = ed->next;
		else
			prev->next = ed->next;

		ed->next = NULL;
		ed_free(ed);

		act_char("Extra description deleted.", ch);
		return TRUE;
	}

	if (!str_cmp(command, "show")) {
		BUFFER *output;

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		output = buf_new(0);
		buf_printf(output, BUF_END, "Keyword:     [%s]\n", ed->keyword);
		buf_append(output, "Description:\n");
		mlstr_dump(output, str_empty, &ed->description, DUMP_LEVEL(ch));
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if (!str_cmp(command, "format")) {
		const char **p;

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		MLSTR_FOREACH(p, &ed->description)
			*p = format_string(*p);

		act_char("Extra description formatted.", ch);
		return TRUE;
	}

	OLC_ERROR("'OLC EXD'");
}

bool
olced_flag(CHAR_DATA *ch, const char *argument, olc_cmd_t* cmd, flag_t *pflag)
{
	const flaginfo_t *flag_table;
	const flaginfo_t *f;
	flag_t ttype;
	const char *tname;

	if (!cmd->arg1) {
		act_puts("$t: $T: Table of values undefined (report it to implementors).",
			 ch, OLCED(ch)->name, cmd->name,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (IS_NULLSTR(argument)
	||  !str_cmp(argument, "?")) {
		show_flags(ch, cmd->arg1);
		return FALSE;
	}

	flag_table = cmd->arg1;
	tname = flag_table->name;
	ttype = flag_table->bit;
	flag_table++;

	switch (ttype) {
	case TABLE_BITVAL: {
		flag_t marked = 0;

		/*
		 * Accept multiple flags.
		 */
		for (;;) {
			char word[MAX_INPUT_LENGTH];

			argument = one_argument(argument, word, sizeof(word));

			if (word[0] == '\0')
				break;

			if ((f = flag_lookup(flag_table, word)) == NULL) {
				act_puts("Usage: $t <flag(s)>",
					 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
				act_puts("Type '$t ?' for a list of acceptable flags.",
					 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return FALSE;
			}
			if (!f->settable) {
				act_puts3("$t: $T: '$U': flag is not settable.",
					  ch, OLCED(ch)->name, cmd->name,
					  f->name,
					  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					  POS_DEAD);
				continue;
			}
			SET_BIT(marked, f->bit);
		}

		if (cmd->validator && !cmd->validator(ch, &marked))
			return FALSE;

		if (marked) {
			flag_t fset = ~(*pflag) & marked;
			flag_t freset = *pflag & marked;
			TOGGLE_BIT(*pflag, marked);
			if (fset) {
				act_puts3("$t: $T: '$U': flag(s) set.",
					  ch, OLCED(ch)->name, cmd->name,
					  flag_string(cmd->arg1, fset),
					  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					  POS_DEAD);
			}
			if (freset) {
				act_puts3("$t: $T: '$U': flag(s) reset.",
					  ch, OLCED(ch)->name, cmd->name,
					  flag_string(cmd->arg1, freset),
					  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					  POS_DEAD);
			}
			return TRUE;
		}
		return FALSE;

		/* NOT REACHED */
	}

	case TABLE_INTVAL:
		if ((f = flag_lookup(flag_table, argument)) == NULL) {
			act_puts("Usage: $t <value>",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			act_puts("Type '$t ?' for a list of acceptable values.",				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		if (!f->settable) {
			act_puts3("$t: $T: '$U': value is not settable.",
				  ch, OLCED(ch)->name, cmd->name, f->name,
				  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		if (cmd->validator && !cmd->validator(ch, &f->bit))
			return FALSE;

		*pflag = f->bit;
		act_puts3("$t: $T: '$U': Ok.",
			  ch, OLCED(ch)->name, cmd->name, f->name,
			  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return TRUE;
		/* NOT REACHED */

	default:
		act_puts3("$t: $T: $U: ",
			  ch, OLCED(ch)->name, cmd->name, tname,
			  TO_CHAR | ACT_NOTRANS | ACT_NOUCASE | ACT_NOLF,
			  POS_DEAD);
		act_puts("table type $j unknown (report it to implementors).",
			 ch, (const void *) ttype, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
		/* NOT REACHED */
	}
}

bool
olced_dice(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd, int *pdice)
{
	int num, type, bonus;
	char* p;

	if (argument[0] == '\0')
		goto bail_out;

	num = strtol(argument, &p, 0);
	if (num < 1 || *p != 'd')
		goto bail_out;

	type = strtol(p+1, &p, 0);
	if (type < 1 || *p != '+')
		goto bail_out;

	bonus = strtol(p+1, &p, 0);
	if (bonus < 0 || *p != '\0')
		goto bail_out;

	pdice[DICE_NUMBER] = num;
	pdice[DICE_TYPE]   = type;
	pdice[DICE_BONUS]  = bonus;

	act_puts("$t set to $T.",
		 ch, cmd->name, format_dice(pdice),
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return TRUE;

bail_out:
	act_puts("Usage: $t <number>d<type>+<bonus>",
		 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

bool
olced_rulecl(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd, lang_t *l)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int rulecl;

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (argument[0] == '\0')
		OLC_ERROR("'OLC RULECLASS'");

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		act_puts("$t: $T: unknown rule class",
			 ch, OLCED(ch)->name, arg,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (!str_prefix(arg2, "implicit")) {
		return olced_str(ch, argument, cmd,
				 &l->rules[rulecl].file_impl);
	}

	if (!str_prefix(arg2, "explicit")) {
		return olced_str(ch, argument, cmd,
				 &l->rules[rulecl].file_expl);
	}

	if (!str_prefix(arg2, "flags")) {
		return olced_flag(ch, argument, cmd,
				    &l->rules[rulecl].rcl_flags);
	}

	OLC_ERROR("'OLC RULECLASS'");
}

bool
olced_vform_add(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd __attribute__((unused)), rule_t *r)
{
	char arg[MAX_INPUT_LENGTH];
	int fnum;

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0'
	||  !is_number(arg)
	||  (fnum = atoi(arg)) < 0)
		OLC_ERROR("'OLC VFORM'");

	rule_form_add(r, fnum, argument);
	act_char("Form added.", ch);
	return TRUE;
}

bool
olced_vform_del(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd __attribute__((unused)), rule_t *r)
{
	char arg[MAX_INPUT_LENGTH];
	int fnum;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg) || (fnum = atoi(arg)) < 0)
		OLC_ERROR("'OLC VFORM'");

	rule_form_del(r, fnum);
	act_char("Form deleted.", ch);
	return TRUE;
}

bool
olced_ival(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd, int *pInt)
{
	if (is_number(argument))
		return olced_number(ch, argument, cmd, pInt);
	return olced_flag(ch, argument, cmd, pInt);
}

bool
olced_gender(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd, mlstring *g)
{
	char arg[MAX_INPUT_LENGTH];
	const char *p;

	p = one_argument(argument, arg, sizeof(arg));
	    one_argument(p, arg, sizeof(arg));

	if (flag_svalue(cmd->arg1, arg) < 0) {
		act_char("Usage: gender <lang> <value>", ch);
		act_char("Valid values are:", ch);
		show_flags(ch, cmd->arg1);
		return FALSE;
	}

	if (!str_cmp(arg, "?")) {
		act_char("Valid values are:", ch);
		show_flags(ch, cmd->arg1);
		return FALSE;
	}

	if (!mlstr_edit(g, argument)) {
		act_puts("Usage: $t <lang> <value>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Usage: $t reset",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_addaffect(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd __attribute__((unused)),
		int level, AFFECT_DATA **ppaf)
{
	const char *type = str_empty;
	const flaginfo_t *f;
	where_t *w;
	vo_t location;
	int modifier = 0;
	flag_t where;
	flag_t bitvector = 0;
	flaginfo_t *loctbl;
	AFFECT_DATA *paf;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "type")) {
		skill_t *sk;

		argument = one_argument(argument, arg, sizeof(arg));
		sk = skill_search(arg, ST_ALL);
		if (sk == NULL) {
			BUFFER *output = buf_new(0);
			buf_append(output, "Valid affect skill types are:\n");
			skills_dump(output, ST_ALL);
			page_to_char(buf_string(output), ch);
			buf_free(output);
			return FALSE;
		}

		type = gmlstr_mval(&sk->sk_name);
		argument = one_argument(argument, arg, sizeof(arg));
	}
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC ADDAFFECT'");

	/*
	 * set `w' and `where'
	 */
	if (!str_cmp(arg, "none")) {
		w = NULL;
		where = TO_AFFECTS;
		loctbl = apply_flags;
	} else {
		if ((f = flag_lookup(affect_where_types + 1, arg)) == NULL
		||  !f->settable) {
			act_char("Valid where types are:", ch);
			show_flags(ch, affect_where_types);
			return FALSE;
		}

		where = f->bit;
		if ((w = where_lookup(where)) == NULL) {
			act_puts("$t: not in where_table.",
				 ch, flag_string(affect_where_types, where),
				 NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
				 POS_DEAD);
			return FALSE;
		}
		loctbl = w->loc_table;
	}

	/*
	 * set `location' and initialize `modifier'
	 */
	switch (where) {
	case TO_SKILLS: {
		skill_t *sk;

		if ((sk = skill_search(arg2, ST_ALL)) == NULL) {
			BUFFER *output = buf_new(0);
			buf_append(output, "Valid skills are:\n");
			skills_dump(output, ST_ALL);
			page_to_char(buf_string(output), ch);
			buf_free(output);
			return FALSE;
		}

		location.s = gmlstr_mval(&sk->sk_name);
		argument = one_argument(argument, arg2, sizeof(arg2));
		break;
	}

	default:
		if (!str_cmp(arg2, "none")) {
			if (where == TO_RESISTS) {
				act_char("Specify damage type.", ch);
				return FALSE;
			}
			INT(location) = APPLY_NONE;
			modifier = -1;
		} else {
			if (loctbl == NULL) {
				act_puts("$t: w->loc_table is NULL.",
				    ch, flag_string(affect_where_types, where),
				    NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
				    POS_DEAD);
				return FALSE;
			}
			if ((INT(location) = flag_svalue(loctbl, arg2)) < 0) {
				act_char("Valid locations are:", ch);
				show_flags(ch, loctbl);
				return FALSE;
			}
			argument = one_argument(argument, arg2, sizeof(arg2));
		}
		break;
	}

	/*
	 * set `modifier'
	 */
	if (modifier < 0) {
		/*
		 * do not want modifier to be set
		 */
		modifier = 0;
	} else {
		if (!is_number(arg2))
			OLC_ERROR("'OLC ADDAFFECT'");

		modifier = atoi(arg2);
	}

	/*
	 * set `bitvector'
	 */
	if (w != NULL
	&&  w->bit_table != NULL
	&&  argument[0] != '\0'
	&&  (bitvector = flag_value(w->bit_table, argument)) == 0) {
		act_puts("Valid '$t' bitaffect flags are:",
			 ch, flag_string(affect_where_types, where), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		show_flags(ch, w->bit_table);
		return FALSE;
	}

	paf = aff_new(where, type);
	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
		paf->location.s = str_dup(location.s);
		break;
	default:
		paf->location = location;
		break;
	}
	paf->modifier   = modifier;
	paf->duration   = -1;
	paf->bitvector  = bitvector;
	paf->level      = level;
	paf->next       = *ppaf;
	*ppaf		= paf;
	act_char("Affect added.", ch);
	return TRUE;
}

bool
olced_delaffect(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd __attribute__((unused)), AFFECT_DATA **ppaf)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_prev = NULL;
	char arg[MAX_INPUT_LENGTH];
	int num;

	one_argument(argument, arg, sizeof(arg));

	if (!is_number(arg)) {
		act_char("Usage: delaffect <num>", ch);
		return FALSE;
	}

	if ((num = atoi(arg)) < 0) {
		act_puts("$t: affect number should be > 0.",
			 ch, OLCED(ch)->name, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	for (paf = *ppaf; paf != NULL && num--; paf_prev = paf, paf = paf->next)
		;

	if (paf == NULL) {
		int n = atoi(arg);
		act_puts("$T: affect $j not found.",
			 ch, (const void *) n, OLCED(ch)->name,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (paf_prev == NULL)
		*ppaf = (*ppaf)->next;
	else
		paf_prev->next = paf->next;

	aff_free(paf);
	act_char("Affect removed.", ch);
	return TRUE;
}

bool
olced_resists(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd,
	      int16_t *resists)
{
	char arg[MAX_INPUT_LENGTH];
	int res;
	bool is_num;

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		act_puts("Usage: $t <damclass> [<number> | undef]",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	res = flag_svalue(dam_classes, arg);
	argument = one_argument(argument, arg, sizeof(arg));
	if (!((is_num = is_number(arg)) || !str_cmp(arg, "undef"))
	||  res < 0 || res == DAM_NONE)
		return olced_resists(ch, str_empty, cmd, resists);

	if (is_num)
		resists[res] = atoi(arg);
	else
		resists[res] = RES_UNDEF;
	act_char("Resistance set.", ch);
	return TRUE;
}

int
olced_one_trig(CHAR_DATA *ch, const char *arg, const char *argument,
	       olc_cmd_t *cmd, trig_t *trig)
{
	mprog_t *mp;

	/*
	 * lookup inline mprog
	 */
	if ((mp = mprog_lookup(trig->trig_prog)) == NULL) {
		act_puts("$t: $T: no such mprog.",
		     ch, OLCED(ch)->name, trig->trig_prog,
		     TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (!str_prefix(arg, "show")) {
		BUFFER *buf = buf_new(-1);

		buf_append(buf, strdump(mp->text, DL_COLOR));
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return FALSE;
	}

	if (!str_prefix(arg, "edit")) {
		OLCED2(ch) = olced_lookup(ED_MPROG);
		ch->desc->pEdit2 = mp;
		string_append(ch, &mp->text);
		return mp->name[0] == '@';
	}

	if (!str_prefix(arg, "compile")) {
		MPROG_COMPILE(ch, mp);
		return FALSE;
	}

	if (!str_prefix(arg, "flags")) {
		void *save_arg1 = cmd->arg1;

		cmd->arg1 = mprog_flags;
		olced_flag(ch, argument, cmd, &mp->flags);
		cmd->arg1 = save_arg1;

		/* flags are not saved */
		return FALSE;
	}

	return -1;
}

void
olced_remove_one_trig(CHAR_DATA *ch, trig_t *trig)
{
	mprog_t *mp;

	if ((mp = mprog_lookup(trig->trig_prog)) == NULL) {
		act_puts("$t: warning: $T: no such mprog.",
		     ch, OLCED(ch)->name, trig->trig_prog,
		     TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	} else if (mp->name[0] == '@') {
		/*
		 * mpcode will remain in memory till mod_mpc is
		 * reloaded
		 */
		c_delete(&mprogs, trig->trig_prog);
	}
}

/*
 * NOTE: keep this in sync with pull_one_trigger() in sog/trig.c
 */
static bool
trig_check_arg(CHAR_DATA *ch, int trig_type, const char *argument)
{
	int number;

	if (HAS_TEXT_ARG(trig_type)
	||  HAS_OBJ_ARG(trig_type)
	||  HAS_CMD_ARG(trig_type))
		return TRUE;

	if (HAS_EXIT_ARG(trig_type)) {
		char arg[MAX_STRING_LENGTH];

		for (argument = one_argument(argument, arg, sizeof(arg));
		     arg[0] != '\0';
		     argument = one_argument(argument, arg, sizeof(arg))) {
			int i;
			bool found;

			if (!str_cmp(arg, "portal") || !str_cmp(arg, "all"))
				continue;

			found = FALSE;
			for (i = 0; i < MAX_DIR; i++) {
				if (!str_cmp(arg, dir_name[i])) {
					found = TRUE;
					break;
				}
			}
			if (found)
				continue;

			act_char("Trigger argument should be 'all', 'portal', or exit direction.", ch);
			return FALSE;
		}

		return TRUE;
	}

	if (!is_number(argument)
	||  (number = atoi(argument)) < 0
	||  (trig_type != TRIG_MOB_BRIBE && number > 100)) {
		if (trig_type == TRIG_MOB_BRIBE) {
			act_char("Trigger argument should be non-negative integer number.", ch);
		} else {
			act_char("Trigger argument should be integer number in range 0..100.", ch);
		}
		return FALSE;
	}

	return TRUE;
}

bool
olced_trig(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd,
	   varr *v, int mp_type, int vnum, void *vo)
{
	int num;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	trig_t *trig;
	int rv;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC TRIG'");
		return FALSE;
	}

	if (!str_prefix(arg, "add")) {
		mprog_t *mp;
		int trig_type;

		argument = one_argument(argument, arg2, sizeof(arg2));
		if (arg2[0] == '\0') {
			return olced_trig(
			    ch, str_empty, cmd, v, vnum, mp_type, vo);
		}

		if (!str_cmp(arg2, "?")) {
			show_flags(ch, mptrig_types);
			return FALSE;
		}

		if ((trig_type = flag_value(mptrig_types, arg2)) < 0) {
			act_char("Invalid trigger type.", ch);
			act_char("Use 'trigadd ?' for list of triggers.", ch);
			return FALSE;
		}

		argument = first_arg(argument, arg2, sizeof(arg2), FALSE);
		if (arg2[0] == '\0' || !strcmp(arg2, "@")) {
			const char *mp_name;
			int trignum = c_size(v);

			if (!trig_check_arg(ch, trig_type, argument))
				return FALSE;

			do {
				mp_name = genmpname_vnum(
				    mp_type, vnum, trignum++);
			} while ((mp = c_insert(&mprogs, mp_name)) == NULL);

			mp->name = str_dup(mp_name);
			mp->type = mp_type;

			trig = trig_new(v, trig_type);
			trig->trig_prog = str_qdup(mp->name);
			log_setchar(ch);
			trig_set_arg(trig, str_dup(argument));
			log_unsetchar();

			if (mp_type == MP_T_ROOM)
				x_room_add(vo);
			act_puts("Inline trigger [$j] added.",
				 ch, (const void *) varr_index(v, trig), NULL,
				 TO_CHAR, POS_DEAD);
			return TRUE;
		}

		if ((mp = mprog_lookup(arg2)) == NULL) {
			act_puts("$t: $T: no such mprog.",
			    ch, OLCED(ch)->name, arg2,
			    TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		if (mp->name[0] == '@') {
			act_puts("$t: $T: invalid mprog name.",
			    ch, OLCED(ch)->name, mp->name,
			    TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		if (!trig_check_arg(ch, trig_type, argument))
			return FALSE;

		trig = trig_new(v, trig_type);
		trig->trig_prog = str_qdup(mp->name);
		log_setchar(ch);
		trig_set_arg(trig, str_dup(argument));
		log_unsetchar();

		if (mp_type == MP_T_ROOM)
			x_room_add(vo);
		act_puts("Trigger [$j] added.",
			 ch, (const void *) varr_index(v, trig), NULL,
			 TO_CHAR, POS_DEAD);
		return TRUE;
	}

	/*
	 * lookup trigger
	 */
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (!is_number(arg2))
		return olced_trig(ch, str_empty, cmd, v, vnum, mp_type, vo);

	num = atoi(arg2);
	if ((trig = varr_get(v, num)) == NULL) {
		act_puts("$t: $T: no such trigger.",
		    ch, OLCED(ch)->name, arg2,
		    TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(arg, "delete")) {
		olced_remove_one_trig(ch, trig);
		if (mp_type == MP_T_ROOM)
			x_room_del(vo);
		varr_edelete(v, trig);
		act_char("Ok.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "arg")) {
		log_setchar(ch);
		trig_set_arg(trig, str_dup(argument));
		log_unsetchar();

		act_char("Trigger arg set.", ch);
		return TRUE;
	}

	if ((rv = olced_one_trig(ch, arg, argument, cmd, trig)) < 0)
		return olced_trig(ch, str_empty, cmd, v, mp_type, vnum, vo);
	return rv;
}

bool
olced_damtype(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd,
	      const char **dt)
{
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_puts("Usage: $t <damage message>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Usage: $t ?",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(arg, "?")) {
		BUFFER *output = buf_new(0);
		skills_dump(output, ST_DAMTYPE);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if ((sk = skill_search(arg, ST_DAMTYPE)) == NULL) {
		act_puts("$t: $T: unknown damage class.",
			 ch, OLCED(ch)->name, arg,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(*dt);
	*dt = str_qdup(gmlstr_mval(&sk->sk_name));
	act_char("Damage type set.", ch);
	return TRUE;
}

VALIDATE_FUN(validate_filename)
{
	if (strpbrk(arg, "/")) {
		act_puts("$t: Invalid characters in file name.",
			 ch, OLCED(ch)->name, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_room_vnum)
{
	int vnum = *(const int *) arg;

	if (vnum && get_room_index(vnum) == NULL) {
		act_puts("OLC: $j: no such room.",
			 ch, (const void *) vnum, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}

VALIDATE_FUN(validate_skill_spec)
{
	const char *spn = (const char *) arg;
	C_STRKEY_CHECK(OLCED(ch)->name, &specs, spn);
	return TRUE;
}

VALIDATE_FUN(validate_funname)
{
	const char *name = (const char*) arg;

	if (strpbrk(name, " \t")) {
		act_puts("$t: $T: illegal character in command name.",
			 ch, OLCED(ch)->name, name,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}
	return TRUE;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
OLC_FUN(show_commands)
{
	BUFFER *	output;
	int		col;

	output = buf_new(0);

	col = 0;
	for (cmd = OLCED(ch)->cmd_table+FUN_FIRST; cmd->name; cmd++) {
		if (IS_NULLSTR(cmd->name))
			continue;
		buf_printf(output, BUF_END, "%-15.15s", cmd->name);
		if (++col % 5 == 0)
			buf_append(output, "\n");
	}
	if (col % 5 != 0)
		buf_append(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(show_version)
{
	act_char(VERSION"\n"
		 AUTHOR	"\n"
		 DATE	"\n"
		 CREDITS, ch);

	return FALSE;
}

AREA_DATA *
get_edited_area(CHAR_DATA *ch)
{
	int vnum;
	olced_t *olced = OLCED(ch);
	void *p = ch->desc->pEdit;

	if (!olced)
		return NULL;

	if (olced->id == ED_AREA)
		return p;

	if (olced->id == ED_HELP)
		return ((HELP_DATA*) p)->area;

	if (olced->id == ED_ROOM)
		return ch->in_room->area;

	if (olced->id == ED_OBJ)
		vnum = ((OBJ_INDEX_DATA*) p)->vnum;
	else if (olced->id == ED_MOB)
		vnum = ((MOB_INDEX_DATA*) p)->vnum;
	else
		return NULL;

	return area_vnum_lookup(vnum);
}

void
edit_done(DESCRIPTOR_DATA *d)
{
	d->pEdit = NULL;
	d->olced = NULL;
}

/*
 * lookup OLC editor by id
 */
olced_t *
olced_lookup(const char * id)
{
	olced_t *olced;

	if (IS_NULLSTR(id))
		return NULL;

	for (olced = olced_table; olced->id; olced++)
		if (!str_prefix(id, olced->id))
			return olced;
	return NULL;
}

void
olc_printf(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (ch) {
		act_puts("$t", ch, buf, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	} else
		printlog(LOG_INFO, buf);
	wiznet("$t", ch, buf, WIZ_OLC, 0, 0);
}

bool
olc_trusted(CHAR_DATA *ch, int min_sec)
{
	int sec = ch ? (IS_NPC(ch) ? 0 : PC(ch)->security) : 9;

	if (sec < min_sec) {
		olc_printf(ch, "Insufficient security.");
		return FALSE;
	}

	return TRUE;
}

FILE *
olc_fopen(const char *path, const char *file, CHAR_DATA *ch, int min_sec)
{
	FILE *fp;

	if (min_sec > 0 && !olc_trusted(ch, min_sec))
		return NULL;

	if ((fp = dfopen(path, file, "w")) == NULL) {
		olc_printf(ch, "%s%c%s: %s",
			   path, PATH_SEPARATOR, file, strerror(errno));
		return NULL;
	}

	return fp;
}

BUFFER *
show_mob_resets(int vnum)
{
	BUFFER *buf = NULL;
	ROOM_INDEX_DATA *room;

	C_FOREACH (ROOM_INDEX_DATA *, room, &rooms) {
		int j = 0;
		RESET_DATA *reset;

		for (reset = room->reset_first; reset != NULL; reset = reset->next) {
			j++;

			if (reset->command != 'M'
			||  reset->arg1 != vnum)
				continue;

			if (!buf)
				buf = buf_new(0);

			buf_printf(buf, BUF_END, "        room %d, reset %d\n",
			    room->vnum, j);
		}
	}

	return buf;
}

BUFFER *
show_obj_resets(int vnum)
{
	BUFFER *buf = NULL;
	ROOM_INDEX_DATA *room;

	C_FOREACH (ROOM_INDEX_DATA *, room, &rooms) {
		int j = 0;
		RESET_DATA *reset;

		for (reset = room->reset_first; reset != NULL; reset = reset->next) {
			bool found = FALSE;

			j++;
			switch (reset->command) {
			case 'P':
				if (reset->arg3 == vnum)
					found = TRUE;

				/* FALLTHRU */

			case 'O':
			case 'G':
			case 'E':
				if (reset->arg1 == vnum)
					found = TRUE;
				break;
			}

			if (!found)
				continue;

			if (!buf)
				buf = buf_new(0);

			buf_printf(buf, BUF_END, "        room %d, reset %d\n",
			    room->vnum, j);
		}
	}

	return buf;
}

void
show_resets(CHAR_DATA *ch, int vnum, const char *xxx,
	    BUFFER *(*show_xxx_resets)(int))
{
	BUFFER *buf = show_xxx_resets(vnum);
	if (!buf) {
		act_puts3("No resets for $t vnum $J found.",
			  ch, xxx, NULL, (const void *) vnum,
			  TO_CHAR, POS_DEAD);
		return;
	}

	buf_printf(buf, BUF_START, "Resets for %s vnum %d:\n", xxx, vnum);
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}

const char *
format_dice(int *pdice)
{
	static char buf[MAX_STRING_LENGTH];
	snprintf(buf, sizeof(buf), "%dd%d+%d",
		 pdice[DICE_NUMBER], pdice[DICE_TYPE], pdice[DICE_BONUS]);
	return buf;
}

void
dump_resists(BUFFER *buf, int16_t *resists)
{
	int i, j;
	bool found = FALSE;

	for (i = 0, j = 0; i < MAX_RESIST; i++) {
		const char *dam_class;

		if (resists[i] == RES_UNDEF)
			continue;

		if (!found) {
			buf_append(buf, "Resists:\n");
			found = TRUE;
		}

		dam_class = flag_string(dam_classes, i);
		if (strlen(dam_class) > 7) {
			buf_printf(buf, BUF_END, "\t%s\t%d%%",
			    dam_class, resists[i]);
		} else {
			buf_printf(buf, BUF_END, "\t%s\t\t%d%%",
			    dam_class, resists[i]);
		}

		if (++j % 3 == 0)
			buf_append(buf, "\n");
	}

	if (j % 3 != 0)
		buf_append(buf, "\n");
}

/******************************************************************************
 * local functions
 */

/* lookup cmd function by name */
static olc_cmd_t *
olc_cmd_lookup(olc_cmd_t *cmd_table, const char *name)
{
	for (; cmd_table->name; cmd_table++) {
		if (!str_prefix(name, cmd_table->name))
			return cmd_table;
	}

	return NULL;
}

static const char *
help_topics[FUN_MAX] =
{
	"'OLC CREATE'",
	"'OLC EDIT'",
	"'OLC ASAVE'",
	str_empty,
	"'OLC ASHOW'",
	"'OLC ALIST'"
};

static void
do_olc(CHAR_DATA *ch, const char *argument, int fun)
{
	char command[MAX_INPUT_LENGTH];
	olced_t *olced;
	OLC_FUN *olc_fun;

	if (ch != NULL && IS_NPC(ch))
		return;

	argument = one_argument(argument, command, sizeof(command));
	if ((olced = olced_lookup(command)) == NULL
	||  (olc_fun = olced->cmd_table[fun].olc_fun) == NULL) {
		if (ch != NULL)
			dofun("help", ch, help_topics[fun]);
		return;
	}

	olced->cmd_table[fun].olc_fun(ch, argument, olced->cmd_table+fun);
}
