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
 * $Id: olc.c,v 1.84 1999-11-24 12:01:32 fjoe Exp $
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

#include <sys/types.h>
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

int changed_flags;

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
const char ED_RACE[]	= "race";
const char ED_CLASS[]	= "class";
const char ED_MATERIAL[]= "materials";
const char ED_LIQUID[]	= "liquids";

olced_t olced_table[] = {
	{ ED_AREA,	"AreaEd",	olc_cmds_area	},
	{ ED_ROOM,	"RoomEd",	olc_cmds_room	},
	{ ED_OBJ,	"ObjEd",	olc_cmds_obj	},
	{ ED_MOB,	"MobEd",	olc_cmds_mob	},
	{ ED_MPCODE,	"MPEd",		olc_cmds_mpcode	},
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
#if 0
	{ ED_CMD,	"CmdEd",	olc_cmds_cmd	}, 
	{ ED_SKILL,	"SkillEd",	olc_cmds_skill	},
#endif
	{ NULL }
};

static olc_cmd_t *	olc_cmd_lookup(olc_cmd_t *cmd_table, const char *name);

static void do_olc(CHAR_DATA *ch, const char *argument, int fun);

int _module_load(module_t *m)
{
	varr_foreach(&commands, cmd_load_cb, CC_OLC, m);
	olc_interpret = dlsym(m->dlh, "_olc_interpret");
	if (olc_interpret == NULL)
		wizlog("_module_load: %s", dlerror());
	return 0;
}

int _module_unload(module_t *m)
{
	DESCRIPTOR_DATA *d;

	/* drop all the builders out OLC editors */
	for (d = descriptor_list; d; d = d->next) {
		if (d->olced == NULL)
			continue;

		char_puts("Unloading OLC module.\n", d->character);
		if (d->pString)
			string_add_exit(d->character, FALSE);
		edit_done(d);
	}

	varr_foreach(&commands, cmd_unload_cb, CC_OLC);
	olc_interpret = NULL;
	return 0;
}

bool _olc_interpret(DESCRIPTOR_DATA *d, const char *argument)
{
	char command[MAX_INPUT_LENGTH];
	olc_cmd_t *cmd;
	olced_t *olced;

	if ((olced = d->olced) == NULL)
		return FALSE;

	argument = one_argument(argument, command, sizeof(command));

	if (command[0] == '\0') {
		olced->cmd_table[FUN_SHOW].olc_fun(d->character, argument,
						   olced->cmd_table+FUN_SHOW);
		return TRUE;
	}

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
			char_printf(ch, "%s: %s is locking this editor "
					"right now.\n",
				    d->olced->name,
				    vch->name);
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
	char_puts("Spell it out.\n", ch);
	return FALSE;
}

OLC_FUN(olced_strkey)
{
	void *p, *q;
	char arg[MAX_INPUT_LENGTH];
	olced_strkey_t *o;

	one_argument(argument, arg, sizeof(arg));
	if (IS_NULLSTR(arg)) {
		char_printf(ch, "Syntax: %s string\n", cmd->name);
		return FALSE;
	}

	if (olced_busy(ch, OLCED(ch)->id, NULL, NULL))
		return FALSE;

	p = ch->desc->pEdit;
	if (!str_cmp(*(const char **)p, arg)) {
		char_puts("Ok.\n", ch);
		return FALSE;
	}

	o = (olced_strkey_t *) cmd->arg1;
	if ((q = hash_insert(o->h, arg, p)) == NULL) {
		char_printf(ch, "%s: %s: duplicate name.\n",
			    OLCED(ch)->name, arg);
		return FALSE;
	}

	if (o->path) {
		d2rename(o->path, strkey_filename(*(const char**) p),
			 o->path, strkey_filename(*(const char**) q));
	}

	hash_delete(o->h, p);
	ch->desc->pEdit = q;

	free_string(*(const char **) q);
	*(const char **) q = str_dup(arg);
	char_puts("Ok.\n", ch);
	return TRUE;
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
		char_printf(ch, "Syntax: %s number\n", cmd->name);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, &val))
		return FALSE;

	*pInt = val;
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_name(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, const char **pStr)
{
	bool changed;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_printf(ch, "Syntax: %s string\n", cmd->name);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	changed = FALSE;
	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if (!str_cmp(arg, "all")) {
			char_printf(ch, "%s: %s: Illegal name.\n",
				    OLCED(ch)->name, arg);
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
		char_printf(ch, "Syntax: %s <name>\n"
				"Use '%s ?' for list of valid %ss.\n"
				"Use '%s none' to reset %s.\n",
			    cmd->name, cmd->name, cmd->name,
			    cmd->name, cmd->name);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		free_string(*pStr);
		*pStr = str_empty;
		return TRUE;
	}

	h = cmd->arg1;
	if (!str_cmp(argument, "?")) {
		BUFFER *out = buf_new(-1);
		buf_printf(out, "Valid %ss are:\n", cmd->name);
		strkey_printall(h, out);
		page_to_char(buf_string(out), ch);
		buf_free(out);
		return FALSE;
	}

	if ((p = strkey_search(h, argument)) == NULL) {
		char_printf(ch, "'%s': unknown %s.\n", argument, cmd->name);
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
		char_printf(ch, "Syntax: %s string\n", cmd->name);
		return FALSE;
	}

	if (cmd->validator && !cmd->validator(ch, argument))
		return FALSE;

	free_string(*pStr);
	*pStr = str_dup(argument);
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_str_text(CHAR_DATA *ch, const char *argument,
		    olc_cmd_t *cmd, const char **pStr)
{
	if (argument[0] =='\0') {
		string_append(ch, pStr);
		return FALSE;
	}

	char_printf(ch, "Syntax: %s\n", cmd->name);
	return FALSE;
}

bool olced_mlstr(CHAR_DATA *ch, const char *argument,
		 olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_edit(mlp, argument)) {
		char_printf(ch, "Syntax: %s lang string\n", cmd->name);
		return FALSE;
	}
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_mlstrnl(CHAR_DATA *ch, const char *argument,
		   olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_editnl(mlp, argument)) {
		char_printf(ch, "Syntax: %s lang string\n", cmd->name);
		return FALSE;
	}
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_mlstr_text(CHAR_DATA *ch, const char *argument,
		      olc_cmd_t *cmd, mlstring *mlp)
{
	if (!mlstr_append(ch, mlp, argument)) {
		char_printf(ch, "Syntax: %s lang\n", cmd->name);
		return FALSE;
	}
	return FALSE;
}

static void cb_format(int lang, const char **p, void *arg)
{
	*p = format_string(*p);
}

bool olced_exd(CHAR_DATA *ch, const char* argument,
	       olc_cmd_t *cmd, ED_DATA **ped)
{
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, keyword, sizeof(keyword));

	if (command[0] == '\0' || keyword[0] == '\0') {
		dofun("help", ch, "'OLC EXD'");
		return FALSE;
	}

	if (!str_cmp(command, "add")) {
		ed		= ed_new();
		ed->keyword	= str_dup(keyword);

		if (!mlstr_append(ch, &ed->description, argument)) {
			ed_free(ed);
			dofun("help", ch, "'OLC EXD'");
			return FALSE;
		}

		ed->next	= *ped;
		*ped		= ed;
		char_puts("Extra description added.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "name")) {
		bool changed;
		char arg[MAX_INPUT_LENGTH];

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		changed = FALSE;
		while (TRUE) {
			argument = one_argument(argument, arg, sizeof(arg));
			if (arg[0] == '\0')
				break;

			if (!str_cmp(arg, "none")
			||  !str_cmp(arg, "all")) {
				char_printf(ch, "%s: %s: Illegal keyword.\n",
					    OLCED(ch)->name, arg);
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
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		if (!mlstr_append(ch, &ed->description, argument)) {
			dofun("help", ch, "'OLC EXD'");
			return FALSE;
		}
		return TRUE;
	}

	if (!str_cmp(command, "delete")) {
		ED_DATA *prev = NULL;
		int num;
		char arg[MAX_INPUT_LENGTH];

		num = number_argument(argument, arg, sizeof(arg));
		for (ed = *ped; ed != NULL; prev = ed, ed = ed->next) {
			if (arg[0] && !is_name(arg, ed->keyword))
				continue;
			if (!--num)
				break;
		}

		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		if (prev == NULL)
			*ped = ed->next;
		else
			prev->next = ed->next;

		ed->next = NULL;
		ed_free(ed);

		char_puts("Extra description deleted.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "show")) {
		BUFFER *output;

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		output = buf_new(-1);
		buf_printf(output, "Keyword:     [%s]\n", ed->keyword);
		mlstr_dump(output, "Description: ", &ed->description);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if (!str_cmp(command, "format")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		mlstr_foreach(&ed->description, NULL, cb_format);
		char_puts("Extra description formatted.\n", ch);
		return TRUE;
	}

	dofun("help", ch, "'OLC EXD'");
	return FALSE;
}

bool olced_flag64(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t* cmd, flag64_t *pflag)
{
	const flag_t *flag64_table;
	const flag_t *f;
	flag64_t ttype;
	const char *tname;

	if (!cmd->arg1) {
		char_printf(ch, "%s: %s: Table of values undefined (report it to implementors).\n", OLCED(ch)->name, cmd->name);
		return FALSE;
	}

	if (IS_NULLSTR(argument)
	||  !str_cmp(argument, "?")) {
		show_flags(ch, cmd->arg1);
		return FALSE;
	}

	flag64_table = cmd->arg1;
	tname = flag64_table->name;
	ttype = flag64_table->bit;
	flag64_table++;

	switch (ttype) {
	case TABLE_BITVAL: {
		flag64_t marked = 0;

		/*
		 * Accept multiple flags.
		 */
		for (;;) {
			char word[MAX_INPUT_LENGTH];
	
			argument = one_argument(argument, word, sizeof(word));
	
			if (word[0] == '\0')
				break;
	
			if ((f = flag_lookup(cmd->arg1, word)) == NULL) {
				char_printf(ch, "Syntax: %s flag...\n"
						"Type '%s ?' for a list of "
						"acceptable flags.\n",
						cmd->name, cmd->name);
				return FALSE;
			}
			if (!f->settable) {
				char_printf(ch, "%s: %s: '%s': flag is not "
						"settable.\n",
					    OLCED(ch)->name,
					    cmd->name, f->name);
				continue;
			}
			SET_BIT(marked, f->bit);
		}
	
		if (marked) {
			TOGGLE_BIT(*pflag, marked);
			char_printf(ch, "%s: %s: '%s': flag(s) toggled.\n",
				    OLCED(ch)->name, cmd->name,
				    flag_string(cmd->arg1, marked));
			return TRUE;
		}
		return FALSE;

		/* NOT REACHED */
	}

	case TABLE_INTVAL:
		if ((f = flag_lookup(cmd->arg1, argument)) == NULL) {
			char_printf(ch, "Syntax: %s value\n"
					"Type '%s ?' for a list of "
					"acceptable values.\n",
					cmd->name, cmd->name);
			return FALSE;
		}
		if (!f->settable) {
			char_printf(ch, "%s: %s: '%s': value is not settable.\n",
				    OLCED(ch)->name, cmd->name, f->name);
			return FALSE;
		}
		*pflag = f->bit;
		char_printf(ch, "%s: %s: '%s': Ok.\n",
			    OLCED(ch)->name, cmd->name, f->name);
		return TRUE;
		/* NOT REACHED */

	default:
		char_printf(ch, "%s: %s: %s: table type %d unknown (report it to implementors).\n", OLCED(ch)->name, cmd->name, tname, ttype);
		return FALSE;
		/* NOT REACHED */
	}
}

bool olced_flag32(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t *cmd, flag32_t *psflag)
{
	flag64_t flag = (flag64_t) (*psflag);
	bool retval = olced_flag64(ch, argument, cmd, &flag);
	if (retval)
		*psflag = (flag32_t) flag;
	return retval;
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

	char_printf(ch, "%s set to %dd%d+%d.\n", cmd->name, num, type, bonus);
	return TRUE;

bail_out:
	char_printf(ch, "Syntax: %s <number>d<type>+<bonus>\n", cmd->name);
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

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC RULECLASS'");
		return FALSE;
	}

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		char_printf(ch, "%s: %s: unknown rule class\n",
			    OLCED(ch)->name, arg);
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
		return olced_flag32(ch, argument, cmd,
				    &l->rules[rulecl].rcl_flags);
	}

	dofun("help", ch, "'OLC RULECLASS'");
	return FALSE;
}

bool olced_vform_add(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
{
	char arg[MAX_INPUT_LENGTH];
	int fnum;

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0'
	||  !is_number(arg)
	||  (fnum = atoi(arg)) < 0) {
		dofun("help", ch, "'OLC VFORM'");
		return FALSE;
	}

	vform_add(r->f, fnum, argument);
	char_puts("Form added.\n", ch);
	return TRUE;
}

bool olced_vform_del(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
{
	char arg[MAX_INPUT_LENGTH];
	int fnum;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg) || (fnum = atoi(arg)) < 0) {
		dofun("help", ch, "'OLC FORM'");
		return FALSE;
	}

	vform_del(r->f, fnum);
	char_puts("Form deleted.\n", ch);
	return TRUE;
}

bool olced_ival(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, int *pInt)
{
	if (is_number(argument))
		return olced_number(ch, argument, cmd, pInt);
	return olced_flag32(ch, argument, cmd, pInt);
}

#define CC_RULES_ERR					\
	do {						\
		dofun("help", ch, "'OLC CC_RULES'");	\
		return FALSE;				\
	} while (0);
		
bool
olced_cc_rules(CHAR_DATA *ch, const char *argument,
	       olc_cmd_t *cmd, const char *rcn, varr *v)
{
	char arg[MAX_INPUT_LENGTH];
	char type[MAX_INPUT_LENGTH];
	varr *v2;
	bool add;
	cc_ruleset_t *rs;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		CC_RULES_ERR;

	if (!str_prefix(arg, "show")) {
		BUFFER *buf = buf_new(-1);
		varr_foreach(v, print_cc_ruleset_cb,
			     buf, "obj_wear", "Restrictions:\n");
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return FALSE;
	}

	argument = one_argument(argument, type, sizeof(type));
	if (type[0] == '\0')
		CC_RULES_ERR;

	/*
	 * parse 'order'
	 */
	if (!str_prefix(arg, "order")) {
		if ((rs = cc_ruleset_lookup(v, type)) == NULL) {
			char_printf(ch, "%s: %s: no rules for type '%s' defined.\n",
				    OLCED(ch)->name, cmd->name, type);
			return FALSE;
		}
		
		return olced_flag32(ch, argument, cmd, &rs->order);
	}

	/*
	 * parse 'add' or 'delete'
	 */
	if (!str_prefix(arg, "add"))
		add = TRUE;
	else if (!str_prefix(arg, "delete"))
		add = FALSE;
	else
		CC_RULES_ERR;

	/*
	 * sanity checking for 'xxx type add ...'
	 */
	if (add) {
		cc_rulecl_t *rcl;

		if ((rcl = cc_rulecl_lookup(rcn)) == NULL) {
			char_printf(ch, "%s: %s: %s: unknown cc_rule class.\n",
				    OLCED(ch)->name, cmd->name, rcn);
			return FALSE;
		}

		if (cc_rulefun_lookup(rcl, type) == NULL) {
			char_printf(ch, "%s: %s: %s: unknown type for cc_rule class '%s'.\n",
				    OLCED(ch)->name, cmd->name, type, rcn);
			return FALSE;
		}
	}

	/*
	 * lookup ruleset
	 * if not found build new if adding, bail out if deleting
	 */
	if ((rs = cc_ruleset_lookup(v, type)) == NULL) {
		if (add) {
			rs = varr_enew(v);
			rs->type = str_dup(type);
		} else {
			char_printf(ch, "%s: %s: no rules for type '%s' defined.\n",
				    OLCED(ch)->name, cmd->name, type);
			return FALSE;
		}
	}

	/*
	 * parse 'allow' or 'deny'
	 */
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		CC_RULES_ERR;

	if (!str_cmp(arg, "allow"))
		v2 = &rs->allow;
	else if (!str_cmp(arg, "deny"))
		v2 = &rs->deny;
	else
		CC_RULES_ERR;

	if (add) {
		const char **ps = varr_enew(v2);
		*ps = str_dup(argument);
		char_printf(ch, "%s: %s: arg added.\n",
			    OLCED(ch)->name, cmd->name);
	} else {
		int num;
		void *p;

		argument = one_argument(argument, arg, sizeof(arg));
		if (!is_number(arg))
			CC_RULES_ERR;

		num = atoi(arg);
		p = varr_get(v2, num);
		if (p == NULL) {
			char_printf(ch, "%s: %s: no arg with number '%s' for type '%s'.\n",
				    OLCED(ch)->name, cmd->name, arg, type);
			return FALSE;
		}

		varr_edelete(v2, p);
		if (cc_ruleset_isempty(rs))
			varr_edelete(v, rs);
		char_printf(ch, "%s: %s: arg deleted.\n",
			    OLCED(ch)->name, cmd->name);
	}

	return TRUE;
}

VALIDATE_FUN(validate_filename)
{
	if (strpbrk(arg, "/")) {
		char_printf(ch, "%s: Invalid characters in file name.\n",
			    OLCED(ch)->name);
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_room_vnum)
{
	int vnum = *(int*) arg;

	if (vnum && get_room_index(vnum) == NULL) {
		char_printf(ch, "OLC: %d: no such room.\n", vnum);
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

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
OLC_FUN(show_commands)
{
	BUFFER *	output;
	int		col;

	output = buf_new(-1); 

	col = 0;
	for (cmd = OLCED(ch)->cmd_table+FUN_FIRST; cmd->name; cmd++) {
		if (IS_NULLSTR(cmd->name))
			continue;
		buf_printf(output, "%-15.15s", cmd->name);
		if (++col % 5 == 0)
			buf_add(output, "\n");
	}
	if (col % 5 != 0)
		buf_add(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(show_version)
{
	char_puts(VERSION	"\n"
		  AUTHOR	"\n"
		  DATE		"\n"
		  CREDITS	"\n", ch);

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
	else if (olced->id == ED_MPCODE)
		vnum = ((MPCODE*) p)->vnum;
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

	if (ch)
		char_printf(ch, "%s\n", buf);
	else
		log(buf);
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

