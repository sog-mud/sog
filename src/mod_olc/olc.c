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
 * $Id: olc.c,v 1.130 2001-06-25 16:51:22 fjoe Exp $
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
#include "lang.h"
#include "string_edit.h"
#include "module.h"

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
const char ED_MPCODE[]	= "mpcodes";
const char ED_HELP[]	= "helps";
const char ED_CLAN[]	= "clans";
const char ED_MSG[]	= "msgdb";
const char ED_LANG[]	= "languages";
const char ED_IMPL[]	= "implicit";
const char ED_EXPL[]	= "explicit";
const char ED_SOCIAL[]	= "socials";
const char ED_CMD[]	= "cmds";
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
#if 0
	XXX
	{ ED_MPCODE,	"MPEd",		olc_cmds_mpcode	},
#endif
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

	{ NULL }
};

static olc_cmd_t *	olc_cmd_lookup(olc_cmd_t *cmd_table, const char *name);

static void do_olc(CHAR_DATA *ch, const char *argument, int fun);

const char *skip_commands[] = { "n", "w", "e", "s", "u", "d" };
#define NSKIP_COMMANDS (sizeof(skip_commands) / sizeof(*skip_commands))

int
_module_load(module_t *m)
{
	varr_foreach(&commands, cmd_load_cb, MODULE, m);
	olc_interpret = dlsym(m->dlh, "_olc_interpret");
	if (olc_interpret == NULL)
		log(LOG_INFO, "_module_load(olc): %s", dlerror());
	qsort(skip_commands, NSKIP_COMMANDS, sizeof(*skip_commands), cmpstr);
	return 0;
}

int
_module_unload(module_t *m)
{
	DESCRIPTOR_DATA *d;

	/* drop all the builders out OLC editors */
	for (d = descriptor_list; d; d = d->next) {
		if (d->olced == NULL)
			continue;

		act_char("Unloading OLC module.", d->character);
		if (d->pString)
			string_add_exit(d->character, FALSE);
		edit_done(d);
	}

	varr_foreach(&commands, cmd_unload_cb, MODULE);
	olc_interpret = NULL;
	return 0;
}

bool _olc_interpret(DESCRIPTOR_DATA *d, const char *argument)
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

	if (cmd->olc_fun(d->character, argument, cmd))
		olced->cmd_table[FUN_TOUCH].olc_fun(d->character, str_empty,
						    olced->cmd_table+FUN_TOUCH);

	return TRUE;
}

void do_create(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_CREATE);
}

void do_edit(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_EDIT);
}

void do_asave(CHAR_DATA *ch, const char *argument)
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

void do_alist(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_LIST);
}

void do_ashow(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_SHOW);
}

/*
 * olced_busy -- returns TRUE if there is another character
 *		 is using the same OLC editor
 */
bool olced_busy(CHAR_DATA *ch, const char *id, void *edit, void *edit2)
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
	const char *old_key;
	olced_strkey_t *o;
	void *q;

	one_argument(argument, arg, sizeof(arg));
	if (IS_NULLSTR(arg)) {
		act_puts("Syntax: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, arg))
		return FALSE;

	if (olced_busy(ch, OLCED(ch)->id, NULL, NULL))
		return FALSE;

	old_key = *(const char **) ch->desc->pEdit;
	if (!str_cmp(old_key, arg)) {
		/*
		 * nothing to change
		 */
		act_char("Ok.", ch);
		return FALSE;
	}

	o = (olced_strkey_t *) cmd->arg1;
	if ((q = hash_insert(o->h, arg, ch->desc->pEdit)) == NULL) {
		act_puts("$t: $T: duplicate name.",
			 ch, OLCED(ch)->name, arg,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	free_string(*(const char **) q);
	*(const char **) q = str_dup(arg);

	if (o->path) {
		d2rename(o->path, strkey_filename(old_key, o->ext),
			 o->path, strkey_filename(arg, o->ext));
	}

	hash_delete(o->h, old_key);
	ch->desc->pEdit = hash_lookup(o->h, arg);
	if (ch->desc->pEdit == NULL) {
		act_puts("$t: $T: not found.",
			 ch, OLCED(ch)->name, arg,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		edit_done(ch->desc);
		return FALSE;
	}

	act_char("Ok.", ch);
	return TRUE;
}

bool
_olced_mlstrkey(CHAR_DATA *ch, const char *langname, const char *argument,
		olc_cmd_t *cmd)
{
	lang_t *lang;
	const char **pp;
	const char *old_key;
	void *q;
	olced_strkey_t *o;

	if (IS_NULLSTR(argument)) {
		act_puts("Syntax: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (!str_cmp(langname, "all"))
		lang = NULL;
	else if ((lang = lang_lookup(langname)) < 0) {
		act_puts("$t: $T: unknown language",
			 ch, OLCED(ch)->name, langname,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	q = ch->desc->pEdit;
	old_key = mlstr_mval((mlstring *) q);
	o = (olced_strkey_t *) cmd->arg1;

	if (lang == NULL || lang == VARR_GET(&langs, 0)) {
		/*
		 * gonna change key
		 */
		if (olced_busy(ch, OLCED(ch)->id, NULL, NULL))
			return FALSE;

		if (!str_cmp(old_key, argument)) {
			/*
			 * key remain unchanged
			 *
			 * possibly changing to the same mlstring
			 * but too lazy to check this
			 */

			pp = mlstr_convert((mlstring *) q, lang);
			free_string(*pp);
			*pp = str_dup(argument);
			act_char("Ok.", ch);
			return TRUE;
		}

		if ((q = hash_insert(o->h, argument, q)) == NULL) {
			act_puts("$t: $T: duplicate name.",
				 ch, OLCED(ch)->name, argument,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}
	}

	/*
	 * make the change
	 */
	pp = mlstr_convert((mlstring *) q, lang);
	free_string(*pp);
	*pp = str_dup(argument);

	if (lang == NULL || lang == VARR_GET(&langs, 0)) {
		if (o->path) {
			d2rename(o->path, strkey_filename(old_key, o->ext),
				 o->path, strkey_filename(argument, o->ext));
		}

		hash_delete(o->h, old_key);
		ch->desc->pEdit = hash_lookup(o->h, argument);
		if (ch->desc->pEdit == NULL) {
			act_puts("$t: $T: not found.",
				 ch, OLCED(ch)->name, langname,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			edit_done(ch->desc);
			return FALSE;
		}
	}

	act_char("Ok.", ch);
	return TRUE;
}

OLC_FUN(olced_mlstrkey)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, sizeof(arg));
	return _olced_mlstrkey(ch, arg, argument, cmd);
}

bool olced_number(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t* cmd, int *pInt)
{
	int val;
	char *endptr;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	val = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0') {
		act_puts("Syntax: $t <number>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, &val))
		return FALSE;

	*pInt = val;
	act_char("Ok.", ch);
	return TRUE;
}

bool olced_name(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, const char **pStr)
{
	bool changed;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_puts("Syntax: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	changed = FALSE;
	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if (!str_cmp(arg, "all")) {
			act_puts("$t: $T: Illegal name.",
				 ch, OLCED(ch)->name, arg,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			continue;
		}
		changed = TRUE;
		name_toggle(pStr, arg, ch, OLCED(ch)->name);
	}

	return changed;
}

bool olced_foreign_strkey(CHAR_DATA *ch, const char *argument,
			  olc_cmd_t *cmd, const char **pStr)
{
	hash_t *h;
	void *p;

	if (IS_NULLSTR(argument)) {
		act_puts("Syntax: $t <name>",
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

	h = cmd->arg1;
	if (!str_cmp(argument, "?")) {
		BUFFER *out = buf_new(0);
		buf_printf(out, BUF_END, "Valid %ss are:\n", cmd->name);
		strkey_printall(h, out);
		page_to_char(buf_string(out), ch);
		buf_free(out);
		return FALSE;
	}

	if ((p = strkey_search(h, argument)) == NULL) {
		act_puts("'$t': unknown $T.",
			 ch, argument, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(*pStr);
	*pStr = str_qdup(*(const char**) p);
	return TRUE;
}

bool olced_foreign_mlstrkey(CHAR_DATA *ch, const char *argument,
			    olc_cmd_t *cmd, const char **pStr)
{
	hash_t *h;
	void *p;

	if (IS_NULLSTR(argument)) {
		act_puts("Syntax: $t <name>",
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

	h = cmd->arg1;
	if (!str_cmp(argument, "?")) {
		BUFFER *out = buf_new(0);
		buf_printf(out, BUF_END, "Valid %ss are:\n", cmd->name);
		mlstrkey_printall(h, out);
		page_to_char(buf_string(out), ch);
		buf_free(out);
		return FALSE;
	}

	if ((p = mlstrkey_search(h, argument)) == NULL) {
		act_puts("'$t': unknown $T.",
			 ch, argument, cmd->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(*pStr);
	*pStr = str_qdup(*(const char**) p);
	return TRUE;
}

bool olced_str(CHAR_DATA *ch, const char *argument,
	       olc_cmd_t *cmd, const char **pStr)
{
	if (IS_NULLSTR(argument)) {
		act_puts("Syntax: $t <string>",
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

bool olced_str_text(CHAR_DATA *ch, const char *argument,
		    olc_cmd_t *cmd, const char **pStr)
{
	if (argument[0] =='\0') {
		string_append(ch, pStr);
		return FALSE;
	}

	act_puts("Syntax: $t",
		 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

bool olced_mlstr(CHAR_DATA *ch, const char *argument,
		 olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_edit(mlp, argument)) {
		act_puts("Syntax: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	act_char("Ok.", ch);
	return TRUE;
}

bool olced_mlstrnl(CHAR_DATA *ch, const char *argument,
		   olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_editnl(mlp, argument)) {
		act_puts("Syntax: $t <lang> <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	act_char("Ok.", ch);
	return TRUE;
}

bool olced_mlstr_text(CHAR_DATA *ch, const char *argument,
		      olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_append(ch, mlp, argument)) {
		act_puts("Syntax: $t <lang>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	return FALSE;
}

static MLSTR_FOREACH_FUN(cb_format, lang, p, ap)
{
	*p = format_string(*p);
	return NULL;
}

bool olced_exd(CHAR_DATA *ch, const char* argument,
	       olc_cmd_t *cmd, ED_DATA **ped)
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
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			act_puts("$t: $T: exd keyword not found.",
				 ch, OLCED(ch)->name, keyword,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return FALSE;
		}

		mlstr_foreach(&ed->description, cb_format);
		act_char("Extra description formatted.", ch);
		return TRUE;
	}

	OLC_ERROR("'OLC EXD'");
}

bool olced_flag(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t* cmd, flag_t *pflag)
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
	
			if ((f = flag_lookup(cmd->arg1, word)) == NULL) {
				act_puts("Syntax: $t <flag(s)>",
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
		if ((f = flag_lookup(cmd->arg1, argument)) == NULL) {
			act_puts("Syntax: $t <value>",
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

bool olced_dice(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, int *dice)
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

	dice[DICE_NUMBER] = num;
	dice[DICE_TYPE]   = type;
	dice[DICE_BONUS]  = bonus;

	act_puts("$t set to $T.",
		 ch, cmd->name, format_dice(dice),
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return TRUE;

bail_out:
	act_puts("Syntax: $t <number>d<type>+<bonus>",
		 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

bool olced_rulecl(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t *cmd, lang_t *l)
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

bool olced_vform_add(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
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

bool olced_vform_del(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
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

bool olced_ival(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, int *pInt)
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
		act_char("Syntax: gender <lang> <value>", ch);
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
		act_puts("Syntax: $t <lang> <value>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	act_char("Ok.", ch);
	return TRUE;
}

bool
olced_addaffect(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd,
		int level, AFFECT_DATA **ppaf)
{
	where_t *w;
	vo_t location;
	int modifier = 0;
	flag_t where;
	flag_t bitvector = 0;
	flaginfo_t *loctbl;
	AFFECT_DATA *paf;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg[0] != '\0' && skill_lookup(arg) == NULL) {
		BUFFER *output = buf_new(0);
		buf_append(output, "Valid types are spell/prayer names (listed below) and empty type (''):\n");
		skills_dump(output, -1);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if (arg1[0] == '\0')
		OLC_ERROR("'OLC ADDAFFECT'");

	/*
	 * set `w' and `where'
	 */
	if (!str_cmp(arg1, "none")) {
		w = NULL;
		where = -1;
		loctbl = apply_flags;
	} else {
		if ((where = flag_value(affect_where_types, arg1)) < 0) {
			act_char("Valid locations are:", ch);
			show_flags(ch, affect_where_types);
			return FALSE;
		}

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

		if ((sk = skill_lookup(arg2)) == NULL) {
			BUFFER *output = buf_new(0);
			buf_append(output, "Valid skills are:\n");
			skills_dump(output, -1);
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
			if (where == TO_RESIST) {
				act_char("Specify damage type.", ch);
				return FALSE;
			}
			INT(location) = APPLY_NONE;
			modifier = -1;
		} else {
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
	if (w
	&&  argument[0] != '\0'
	&&  (bitvector = flag_value(w->bit_table, argument)) == 0) {
		act_puts("Valid '$t' bitaffect flags are:",
			 ch, flag_string(affect_where_types, where), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		show_flags(ch, w->bit_table);
		return FALSE;
	}

	paf             = aff_new();
	switch (where) {
	case TO_SKILLS:
	case TO_RACE:
		paf->location.s = str_dup(location.s);
		break;
	default:
		paf->location = location;
		break;
	}
	paf->modifier   = modifier;
	paf->where	= where;
	paf->type       = str_dup(arg);
	paf->duration   = -1;
	paf->bitvector  = bitvector;
	paf->level      = level;
	paf->next       = *ppaf;
	*ppaf		= paf;
	act_char("Affect added.", ch);
	return TRUE;
}

bool
olced_delaffect(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd,
		AFFECT_DATA **ppaf)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_prev = NULL;
	char arg[MAX_INPUT_LENGTH];
	int num;

	one_argument(argument, arg, sizeof(arg));

	if (!is_number(arg)) {
		act_char("Syntax: delaffect <num>", ch);
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
		act_puts("$T: affect $j not found.",
			 ch, (const void *) atoi(arg), OLCED(ch)->name,
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

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		act_puts("Syntax: $t <damclass> <number>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	
	res = flag_svalue(dam_classes, arg);
	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg) || res < 0 || res == DAM_NONE)
		return olced_resists(ch, str_empty, cmd, resists);
	
	resists[res] = atoi(arg);
	act_char("Resistance set.", ch);
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
	int vnum = *(int*) arg;

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
	STRKEY_CHECK(&specs, spn, OLCED(ch)->name); 
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

AREA_DATA *get_edited_area(CHAR_DATA *ch)
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
#if 0
	XXX
	else if (olced->id == ED_MPCODE)
		vnum = ((MPCODE*) p)->vnum;
#endif
	else
		return NULL;

	return area_vnum_lookup(vnum);
}

void edit_done(DESCRIPTOR_DATA *d)
{
	d->pEdit = NULL;
	d->olced = NULL;
}

/*
 * lookup OLC editor by id
 */
olced_t *olced_lookup(const char * id)
{
	olced_t *olced;

	if (IS_NULLSTR(id))
		return NULL;

	for (olced = olced_table; olced->id; olced++)
		if (!str_prefix(id, olced->id))
			return olced;
	return NULL;
}

void olc_printf(CHAR_DATA *ch, const char *format, ...)
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
		log(LOG_INFO, buf);
	wiznet("$t", ch, buf, WIZ_OLC, 0, 0);
}

bool olc_trusted(CHAR_DATA *ch, int min_sec)
{
	int sec = ch ? (IS_NPC(ch) ? 0 : PC(ch)->security) : 9;

	if (sec < min_sec) {
		olc_printf(ch, "Insufficient security.");
		return FALSE;
	}

	return TRUE;
}

FILE *olc_fopen(const char *path, const char *file,
		CHAR_DATA *ch, int min_sec)
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
	int i;
	BUFFER *buf = NULL;

	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;

		for (room = room_index_hash[i]; room; room = room->next) {
			int j = 0;
			RESET_DATA *reset;

			for (reset = room->reset_first; reset != NULL;
							reset = reset->next) {
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
	}

	return buf;
}

BUFFER *
show_obj_resets(int vnum)
{
	int i;
	BUFFER *buf = NULL;

	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;

		for (room = room_index_hash[i]; room; room = room->next) {
			int j = 0;
			RESET_DATA *reset;

			for (reset = room->reset_first; reset != NULL;
							reset = reset->next) {
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
format_dice(int *dice)
{
	static char buf[MAX_STRING_LENGTH];
	snprintf(buf, sizeof(buf), "%dd%d+%d",
		 dice[DICE_NUMBER], dice[DICE_TYPE], dice[DICE_BONUS]);
	return buf;
}

/******************************************************************************
 * local functions
 */

/* lookup cmd function by name */
static olc_cmd_t *olc_cmd_lookup(olc_cmd_t *cmd_table, const char *name)
{
	for (; cmd_table->name; cmd_table++)
		if (!str_prefix(name, cmd_table->name))
			return cmd_table;
	return NULL;
}

char* help_topics[FUN_MAX] =
{
	"'OLC CREATE'",
	"'OLC EDIT'",
	"'OLC ASAVE'",
	str_empty,
	"'OLC ASHOW'",
	"'OLC ALIST'"
};

static void do_olc(CHAR_DATA *ch, const char *argument, int fun)
{
	char command[MAX_INPUT_LENGTH];
	olced_t *olced;
	OLC_FUN *olc_fun;

	if (ch && IS_NPC(ch))
		return;

	argument = one_argument(argument, command, sizeof(command));
	if ((olced = olced_lookup(command)) == NULL
	||  (olc_fun = olced->cmd_table[fun].olc_fun) == NULL) {
		if (ch) {
        		dofun("help", ch, help_topics[fun]);
		}
        	return;
	}

	olced->cmd_table[fun].olc_fun(ch, argument, olced->cmd_table+fun);
}

